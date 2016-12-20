#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>
#include "rds_vpc.h"
#include "rds_vpc_func.h"

int main(int argc, char *argv[])
{
    RDS * cluster = new RDS((char *) "auroratest");
    return(cluster->create_rds_db(4));
}
