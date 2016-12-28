/**
 * @file mxs951_utfmb4_galera.cpp Set utf8mb4 in the backend and restart Maxscale
 * - for all backend nodes: SET GLOBAL character_set_server = 'utf8mb4'; SET NAMES 'utf8mb4'
 * - restart Maxscale
 * - connect to Maxscale
 */



#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(10);

    Test->tprintf("Set utf8mb4 for backend");
    Test->galera->execute_query_all_nodes((char *) "SET GLOBAL character_set_server = 'utf8mb4';");

    Test->tprintf("Set names to utf8mb4 for backend");
    Test->galera->execute_query_all_nodes((char *) "SET NAMES 'utf8mb4';");

    Test->set_timeout(120);

    Test->tprintf("Restart Maxscale");
    Test->restart_maxscale();

    sleep(10);

    Test->check_maxscale_alive();
    //Test->check_maxscale_processes(0);
    Test->copy_all_logs(); return(Test->global_result);
}

