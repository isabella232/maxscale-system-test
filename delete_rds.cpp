#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <jansson.h>
#include "rds_vpc_func.h"


int main(int argc, char *argv[])
{
    delete_rds_cluster();
}
