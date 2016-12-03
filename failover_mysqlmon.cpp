/**
 * MySQL Monitor Failover Test
 */


#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    TestConnections * test = new TestConnections(argc, argv);

    test->tprintf(" Create the test table and insert some data ");
    test->connect_maxscale();
    test->try_query(test->conn_rwsplit, "CREATE OR REPLACE TABLE test.t1 (id int)");
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test->close_maxscale_connections();

    test->tprintf(" Block all but one node ");
    test->repl->block_node(0);
    test->repl->block_node(1);
    test->repl->block_node(2);

    test->tprintf(" Wait for the monitor to detect it ");
    sleep(15);

    test->tprintf(" Connect and insert should work ");
    char *output = test->ssh_maxscale_output(true, "maxadmin list servers");
    test->tprintf("%s", output);
    free(output);
    test->connect_maxscale();
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test->close_maxscale_connections();

    test->tprintf(" Unblock nodes ");
    test->repl->unblock_node(0);
    test->repl->unblock_node(1);
    test->repl->unblock_node(2);

    test->tprintf(" Wait for the monitor to detect it ");
    sleep(15);

    test->tprintf("Check that we are still using the last node to which we failed over"
                  "to and that the old nodes are in maintenance mode");

    test->connect_maxscale();
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    char maxscale_id[256], real_id[256];
    find_field(test->conn_rwsplit, "SELECT @@server_id", "@@server_id", maxscale_id);
    test->repl->connect();
    find_field(test->repl->nodes[3], "SELECT @@server_id", "@@server_id", real_id);
    test->add_result(strcmp(maxscale_id, real_id) != 0,
                     "@@server_id is different: %s != %s", maxscale_id, real_id);
    test->close_maxscale_connections();

    test->tprintf(" Check that MaxScale is running ");
    test->check_maxscale_alive();

    test->copy_all_logs();
    return test->global_result;
}
