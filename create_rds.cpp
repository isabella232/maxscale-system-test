#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>
#include "rds_vpc_func.h"


int main(int argc, char *argv[])
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
    create_cluster(4);

}
