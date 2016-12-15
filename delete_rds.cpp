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

json_t * get_cluster_nodes(char * json)
{
    json_t * cluster = get_cluster_descr(json);
    json_t * members = json_object_get(cluster, "DBClusterMembers");
    size_t i;
    size_t members_N = json_array_size(members);
    json_t * member;
    json_t * node_names = json_array();

    for (i = 0; i < members_N; i++)
    {
        member = json_array_get(members, i);

        json_array_append(node_names, json_string(get_instance_name(member)));
    }
    return(node_names);
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

int main(int argc, char *argv[])
{
    char * result;
    char * j = NULL;


    int i = execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);

    printf("Exit code is %d \n", i);

    json_t * nodes = get_cluster_nodes(result);
    if (nodes == NULL)
    {
        printf("NULL!!\n");
        exit(0);
    }

    size_t alive_nodes = json_array_size(nodes);
    destroy_nodes(nodes);
    json_decref(nodes);


    do
    {
        printf("Waiting for nodes to be deleted, now %lu nodes are still alive\n", alive_nodes);
        sleep(5);
        execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);
        nodes = get_cluster_nodes(result);
        alive_nodes = json_array_size(nodes);
        json_decref(nodes);

    } while ( alive_nodes > 0);

    execute_cmd((char *) "aws rds delete-db-cluster --db-cluster-identifier=auroratest --skip-final-snapshot", &result);

    do
    {
        printf("Waiting for cluster to be deleted\n");
        sleep(5);
        execute_cmd((char *) "aws rds describe-db-clusters --db-cluster-identifier=auroratest", &result);

    } while (get_cluster_descr(result) != NULL);

    execute_cmd((char *) "aws rds delete-db-subnet-group --db-subnet-group-name maxscaleaurora", &result);

}
