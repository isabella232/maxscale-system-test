#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>

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


int main(int argc, char *argv[])
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
    system(cmd);
}
