#ifndef RDS_VPC_FUNC_H
#define RDS_VPC_FUNC_H

#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>

using namespace std;


int execute_cmd(char * cmd, char ** res);

const char * get_instance_name(json_t * instance);

json_t * get_cluster_descr(char * json);

json_t * get_subnet_descr(char * json);

json_t * get_cluster_nodes(json_t * cluster);

json_t * get_subnets(json_t * subnets_group);

const char * get_subnetgroup(json_t * cluster);

int destroy_nodes(json_t * node_names);

int destroy_subnets(json_t * subnets_names);

int destroy_route_tables(const char * vpc, const char * json);

int detach_and_destroy_gw(const char * json, const char * vpc);

int create_vpc(const char **vpc_id);

int create_subnet(const char * vpc, const char *az, const char *cidr, const char **subnet_id);

int create_subnet_group(const char * subnets);

int create_gw(const char * vpc, const char **gw_id);

int configure_route_table(const char * vpc, const char *gw, const char **rt);

int create_cluster(size_t N);



#endif // RDS_VPC_FUNC_H
