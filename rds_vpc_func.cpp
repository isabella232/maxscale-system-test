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

int get_writer(const char ** writer_name)
{
    char * json;
    execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &json);
    json_t * cluster = get_cluster_descr(json);
    json_t * nodes = json_object_get(cluster, "DBClusterMembers");

    //char * s = json_dumps(nodes, JSON_INDENT(4));
    //puts(s);

    bool writer;
    json_t * node;
    size_t i = 0;

    do
    {
        node = json_array_get(nodes, i);
        writer = json_is_true(json_object_get(node, "IsClusterWriter"));
        i++;
    } while (!writer);
    * writer_name = json_string_value(json_object_get(node, "DBInstanceIdentifier"));

    return(0);
}

int create_rds_cluster(int N)
{
    char cmd[1024];

    const char * vpc;
    const char * subnet1;
    const char * subnet2;
    const char * gw;
    const char * rt;

    printf("Create VPC\n");
    create_vpc(&vpc);
    printf("vpc id: %s\n", vpc);

    printf("Create subnets\n");
    create_subnet(vpc, "eu-west-1b", "172.30.0.0/24", &subnet1);
    create_subnet(vpc, "eu-west-1a", "172.30.1.0/24", &subnet2);

    printf("Create subnets group\n");
    sprintf(cmd, "%s %s", subnet1, subnet2);
    create_subnet_group(cmd);

    printf("Create internet gateway\n");
    create_gw(vpc, &gw);
    printf("Gateway: %s\n", gw);

    printf("Configure route table\n");
    configure_route_table(vpc, gw, &rt);
    printf("Route table: %s\n", rt);

    printf("Create RDS cluster\n");
    return(create_cluster(N));
}

int delete_rds_cluster()
{
    char * result;
    char * j = NULL;
    char cmd[1024];

    printf("Get cluster description\n");
    int i = execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);

    json_t * cluster = get_cluster_descr(result);

    json_t * nodes = get_cluster_nodes(cluster);
    if (nodes == NULL)
    {
        printf("NULL!!\n");
        exit(0);
    }


    size_t alive_nodes = json_array_size(nodes);

    printf("Destroy nodes\n");
    destroy_nodes(nodes);

    do
    {
        printf("Waiting for nodes to be deleted, now %lu nodes are still alive\n", alive_nodes);
        sleep(5);
        execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);
        cluster = get_cluster_descr(result);
        nodes = get_cluster_nodes(cluster);
        alive_nodes = json_array_size(nodes);


    } while ( alive_nodes > 0);

    printf("Destroy cluster\n");
    execute_cmd((char *) "aws rds delete-db-cluster --db-cluster-identifier=auroratest --skip-final-snapshot", &result);

    do
    {
        printf("Waiting for cluster to be deleted\n");
        sleep(5);
        execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);

    } while (get_cluster_descr(result) != NULL);

    printf("Get subnets\n");
    sprintf(cmd, "aws rds describe-db-subnet-groups --db-subnet-group-name %s", get_subnetgroup(cluster));
    execute_cmd(cmd, &result);

    json_t * subnet_group = get_subnet_descr(result);

    const char * vpc = json_string_value(json_object_get(subnet_group, "VpcId"));
    printf("VpcId: %s\n", vpc);

    json_t * subnets = get_subnets(subnet_group);
    printf("Destroy subnets\n");
    destroy_subnets(subnets);

    printf("Destroy subnet group\n");
    sprintf(cmd, "aws rds delete-db-subnet-group --db-subnet-group-name %s", get_subnetgroup(cluster));
    execute_cmd(cmd, &result);

    printf("Get Internet Gateways\n");
    sprintf(cmd, "aws ec2 describe-internet-gateways --filters Name=attachment.vpc-id,Values=%s", vpc);
    execute_cmd(cmd, &result);
    detach_and_destroy_gw(result, vpc);


    printf("Get route tables\n");
    execute_cmd((char *) "aws ec2 describe-route-tables", &result);
    //printf("Destroy route tables\n");
    //destroy_route_tables(vpc, result);
    printf("Destroy vpc\n");
    sprintf(cmd, "aws ec2 delete-vpc --vpc-id=%s", vpc);
    return(system(cmd));
}

int wait_for_nodes(size_t N)
{
    char * result;
    size_t active_nodes = 0;
    size_t i = 0;
    json_t * node;
    char cmd[1024];
    json_t * cluster;
    json_t * nodes;
    json_t * instances;
    json_t * instance;
    json_error_t error;

    do
    {
        printf("Waiting for nodes to be active, now %lu are active\n", active_nodes);
        sleep(5);
        execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);
        cluster = get_cluster_descr(result);
        nodes = get_cluster_nodes(cluster);

        active_nodes = 0;
        json_array_foreach(nodes, i, node)
        {
            sprintf(cmd, "aws rds describe-db-instances --db-instance-identifier=%s", json_string_value(node));
            execute_cmd(cmd, &result);
            instances = json_loads( result, 0, &error );
            if ( !instances )
            {
                fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
                return -1;
            }
            instance = json_array_get(json_object_get(instances, "DBInstances"), 0);
            //puts(json_dumps(instance, JSON_INDENT(4)));
            if (strcmp(json_string_value(json_object_get(instance, "DBInstanceStatus")), "available") == 0)
            {
                active_nodes++;
            }
        }
    } while ( active_nodes != N);

}
