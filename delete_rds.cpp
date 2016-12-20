#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>
#include "rds_vpc_func.h"
#include "rds_vpc.h"


int main(int argc, char *argv[])
{
    RDS * cluster = new RDS((char *) "auroratest");
    cluster->delete_rds_cluster();
}
