/**
 * @file
 * Start MaxScale with 1 master and 2 slaves
 * Connect to MaxScale with Readwritesplit
 * Execute SET @a=1
 * Block first slave
 * Wait until monitor detects it
 * Unblock first slave and block the second slave
 * First slave is not recovered
 */

#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(10);

    Test->connect_maxscale();

    Test->set_timeout(10);
    Test->try_query(Test->conn_rwsplit, (char *) "SET @a=1");
    Test->stop_timeout();
    sleep(30);
    Test->set_timeout(20);
    Test->tprintf("Blocking first slave\n");
    Test->repl->block_node(1);
    Test->stop_timeout();
    sleep(30);
    Test->set_timeout(10);
    Test->tprintf("Unblocking first slave and blocking second slave\n");

    Test->repl->unblock_node(1);
    Test->stop_timeout();
    sleep(30);
    Test->repl->block_node(2);
    Test->stop_timeout();
    sleep(30);
    Test->set_timeout(20);

    char server1_status[256];
    Test->get_maxadmin_param((char *) "show server server2", (char *) "Status", server1_status);

    if (strstr(server1_status, "Running") == NULL)
    {
        Test->add_result(1, "Slave is not recovered, slave status is %s\n", server1_status);
    }

    Test->tprintf("Unblocking second slave\n");
    Test->repl->unblock_node(2);

    Test->check_maxscale_alive();
    Test->copy_all_logs(); return(Test->global_result);
}

