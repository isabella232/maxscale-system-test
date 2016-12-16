#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>
#include "rds_vpc_func.h"


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
