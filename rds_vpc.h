#ifndef RDS_VPC_H
#define RDS_VPC_H

#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>

using namespace std;

class RDS
{
public:
    RDS(char * cluster);

    const char * get_instance_name(json_t * instance);

    json_t * get_cluster();

    json_t * get_cluster_descr(char * json);

    json_t * get_subnets_group_descr(char * json);

    json_t * get_cluster_nodes();

    json_t * get_cluster_nodes(json_t * cluster);

    json_t * get_subnets();

    const char * get_subnetgroup_name();

    int destroy_nodes(json_t * node_names);

    int destroy_subnets();

    int destroy_subnets_group();

    int destroy_route_tables(); // is needed?

    int destroy_vpc();

    int destroy_cluster();

    int detach_and_destroy_gw();

    int create_vpc(const char **vpc_id);

    int create_subnet(const char *az, const char *cidr, const char **subnet_id);

    int create_subnet_group();

    int create_gw(const char **gw_id);

    int configure_route_table(const char **rt);

    int create_cluster();

    int get_writer(const char **writer_name);

    /**
     * @brief create_rds_db Creates RDS DB cluster and all needed stuff (vpc, subnets, gateway, route table, ...)
     * @param cluster_name Name of DB cluster
     * @param N Number of nodes
     * @return 0 in case if success
     */
    int create_rds_db(int N);

    int delete_rds_cluster();

    int wait_for_nodes(size_t N);

    const char * cluster_name_intern;
    size_t N_intern;
    json_t * cluster_intern;
    const char * vpc_id_intern;
    json_t * subnets_intern;
    const char * subnets_group_name_intern;
    const char * rt_intern;
    const char * gw_intern;
    const char * sg_intern;

};

#endif // RDS_VPC_H
