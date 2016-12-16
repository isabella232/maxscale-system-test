#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>
#include "rds_vpc_func.h"

using namespace std;


int execute_cmd(char * cmd, char ** res)
{
    char * result;
    FILE *output = popen(cmd, "r");
    if (output == NULL)
    {
        printf("Error opening ssh %s\n", strerror(errno));
        return -1;
    }
    char buffer[10240];
    size_t rsize = sizeof(buffer);
    result =  (char*)calloc(rsize, sizeof(char));

    while(fgets(buffer, sizeof(buffer), output))
    {
        result = (char*)realloc(result, sizeof(buffer) + rsize);
        rsize += sizeof(buffer);
        strcat(result, buffer);
    }

    * res = result;

    return(pclose(output)/256);
}

const char * get_instance_name(json_t * instance)
{
    json_t * instance_name = json_object_get(instance, "DBInstanceIdentifier");
    return(json_string_value(instance_name));
}

json_t * get_cluster_descr(char * json)
{
    json_t *root;
    json_error_t error;
    char * j;

    root = json_loads( json, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return NULL;
    }

    json_t * clusters = json_object_get(root, "DBClusters");
    return(json_array_get(clusters, 0));
}

json_t * get_subnet_descr(char * json)
{
    json_t *root;
    json_error_t error;
    char * j;

    root = json_loads( json, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return NULL;
    }

    json_t * subnets = json_object_get(root, "DBSubnetGroups");
    return(json_array_get(subnets, 0));
}

json_t * get_cluster_nodes(json_t * cluster)
{
    json_t * members = json_object_get(cluster, "DBClusterMembers");
    size_t members_N = json_array_size(members);
    json_t * member;
    json_t * node_names = json_array();

    for (size_t i = 0; i < members_N; i++)
    {
        member = json_array_get(members, i);
        json_array_append(node_names, json_string(get_instance_name(member)));
    }
    return(node_names);
}

json_t * get_subnets(json_t * subnets_group)
{
    json_t * members = json_object_get(subnets_group, "Subnets");
    size_t members_N = json_array_size(members);
    json_t * member;
    json_t * subnets_names = json_array();

    for (size_t i = 0; i < members_N; i++)
    {
        member = json_array_get(members, i);
        json_array_append(subnets_names, json_object_get(member, "SubnetIdentifier"));
    }
    return(subnets_names);
}

const char * get_subnetgroup(json_t * cluster)
{
    return(json_string_value(json_object_get(cluster, "DBSubnetGroup")));
}

int destroy_nodes(json_t * node_names)
{
    size_t N = json_array_size(node_names);

    char cmd[1024];
    char *res;
    json_t * node;
    for (size_t i = 0; i < N; i++)
    {
        node = json_array_get(node_names, i);
        sprintf(cmd, "aws rds delete-db-instance --skip-final-snapshot --db-instance-identifier=%s", json_string_value(node));
        printf("%s\n", cmd);
        execute_cmd(cmd, &res);
    }
}

int destroy_subnets(json_t * subnets_names)
{
    size_t N = json_array_size(subnets_names);

    char cmd[1024];
    char *res;
    json_t * node;
    for (size_t i = 0; i < N; i++)
    {
        node = json_array_get(subnets_names, i);
        sprintf(cmd, "aws ec2 delete-subnet --subnet-id=%s", json_string_value(node));
        printf("%s\n", cmd);
        execute_cmd(cmd, &res);
    }
}

int destroy_route_tables(const char * vpc, const char * json)
{
    json_t *root;
    json_error_t error;
    char cmd[1024];

    root = json_loads( json, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }

    json_t * route_tables = json_object_get(root, "RouteTables");

    size_t i;
    json_t *route_table;
    const char * rt_id;
    const char * vpc_id;
    json_array_foreach(route_tables, i, route_table)
    {
        rt_id = json_string_value(json_object_get(route_table, "RouteTableId"));
        vpc_id = json_string_value(json_object_get(route_table, "VpcId"));
        if (strcmp(vpc, vpc_id) == 0)
        {
            sprintf(cmd, "aws ec2 delete-route-table --route-table-id %s", rt_id);
            system(cmd);
        }
    }

}

int detach_and_destroy_gw(const char * json, const char * vpc)
{
    json_t *root;
    json_error_t error;
    char cmd[1024];

    root = json_loads( json, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }

    json_t * gws = json_object_get(root, "InternetGateways");
    size_t i;
    json_t * gw;
    const char * gw_id;
    json_array_foreach(gws, i, gw)
    {
        gw_id = json_string_value(json_object_get(gw, "InternetGatewayId"));
        sprintf(cmd, "aws ec2 detach-internet-gateway --internet-gateway-id=%s --vpc-id=%s", gw_id, vpc);
        printf("%s\n", cmd);
        system(cmd);
        sprintf(cmd, "aws ec2 delete-internet-gateway --internet-gateway-id=%s", gw_id);
        printf("%s\n", cmd);
        system(cmd);
    }

}

int create_vpc(const char **vpc_id)
{
    json_t *root;
    json_error_t error;
    char * result;
    char cmd[1024];


    execute_cmd((char *) "aws ec2 create-vpc --cidr-block 172.30.0.0/16", &result);
    root = json_loads( result, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }
    *vpc_id = json_string_value(json_object_get(json_object_get(root, "Vpc"), "VpcId"));

    sprintf(cmd, "aws ec2 modify-vpc-attribute --enable-dns-support --vpc-id %s", *vpc_id);
    system(cmd);
    sprintf(cmd, "aws ec2 modify-vpc-attribute --enable-dns-hostnames --vpc-id %s", *vpc_id);
    system(cmd);

    return 0;
}

int create_subnet(const char * vpc, const char * az, const char * cidr, const char **subnet_id)
{
    json_t *root;
    json_error_t error;
    char * result;
    char cmd[1024];

    sprintf(cmd, "aws ec2 create-subnet --cidr-block %s --availability-zone %s --vpc-id %s", cidr, az, vpc);
    execute_cmd(cmd, &result);
    root = json_loads( result, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }
    *subnet_id = json_string_value(json_object_get(json_object_get(root, "Subnet"), "SubnetId"));

    sprintf(cmd, "aws ec2 modify-subnet-attribute --map-public-ip-on-launch --subnet-id %s", *subnet_id);
    system(cmd);
}

int create_subnet_group(const char * subnets)
{
    char cmd[1024];
    sprintf(cmd, "aws rds create-db-subnet-group --db-subnet-group-name maxscaleaurora --db-subnet-group-description maxscale --subnet-ids %s", subnets);
    return(system(cmd));
}

int create_gw(const char * vpc, const char **gw_id)
{
    char * result;
    char cmd[1024];
    json_error_t error;

    execute_cmd((char *) "aws ec2 create-internet-gateway", &result);
    json_t * root = json_loads( result, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }

    *gw_id = json_string_value(json_object_get(json_object_get(root, "InternetGateway"), "InternetGatewayId"));

    sprintf(cmd, "aws ec2 attach-internet-gateway --internet-gateway-id %s --vpc-id %s", *gw_id, vpc);
    return(system(cmd));
}

int configure_route_table(const char * vpc, const char * gw, const char **rt)
{
    char * result;
    char cmd[1024];
    json_error_t error;

    execute_cmd((char *) "aws ec2 describe-route-tables", &result);
    json_t * root = json_loads( result, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }

    json_t * route_tables = json_object_get(root, "RouteTables");
    size_t i;
    json_t * rtb;
    const char * rt_vpc;

    json_array_foreach(route_tables, i, rtb)
    {
        rt_vpc = json_string_value(json_object_get(rtb, "VpcId"));
        if (strcmp(vpc, rt_vpc) == 0)
        {
            // add route to route table which belongs to give VPC
            *rt = json_string_value(json_object_get(rtb, "RouteTableId"));
            sprintf(cmd, "aws ec2 create-route --route-table-id %s --gateway-id %s --destination-cidr-block 0.0.0.0/0", *rt, gw);
            system(cmd);
        }
    }
}

int create_cluster(size_t N)
{
    char cmd[1024];
    char * result;
    json_error_t error;
    size_t i;

    execute_cmd((char *) "aws rds create-db-cluster --database-name=test --engine=aurora --master-username=skysql --master-user-password=skysqlrds --db-cluster-identifier=auroratest --db-subnet-group-name=maxscaleaurora", &result);
    json_t * root = json_loads( result, 0, &error );
    if ( !root )
    {
        fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
        return -1;
    }
    json_t * cluster = json_object_get(root, "DBCluster");
    json_t * security_groups = json_object_get(cluster, "VpcSecurityGroups");
    json_t * sg;
    const char * sg_id;

    json_array_foreach(security_groups, i, sg)
    {
        sg_id = json_string_value(json_object_get(sg, "VpcSecurityGroupId"));
        printf("Security group %s\n", sg_id);
        sprintf(cmd, "aws ec2 authorize-security-group-ingress --group-id %s --protocol tcp --port 3306 --cidr 0.0.0.0/0", sg_id);
        system(cmd);
    }

    for (size_t i = 0; i < N; i++)
    {
        sprintf(cmd, "aws rds create-db-instance --db-cluster-identifier=auroratest --engine=aurora --db-instance-class=db.t2.medium --publicly-accessible --db-instance-identifier=node%03lu", i);
        printf("%s\n", cmd);
        system(cmd);
    }
    return(0);
}
