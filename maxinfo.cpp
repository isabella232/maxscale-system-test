/**
 * @file maxinfo.cpp maxinfo JSON listener test
 * sends 1000 'status' request to the listener
 */

#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

#include <stdio.h>
#include <string.h>
#include "maxinfo_func.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int iterations = Test->smoke ? 100 : 1000;
    char * result;
    for (int i = 0; i < iterations; i++)
    {
        Test->set_timeout(20);
        result = get_maxinfo((char*) "status", Test);
        if (result != NULL)
        {
            printf("%s\n", result);
            free(result);
        } else {
            Test->add_result(1, "Can't get result from maxinfo\n");
        }
    }

    Test->check_maxscale_alive();

    Test->copy_all_logs(); return(Test->global_result);
}

