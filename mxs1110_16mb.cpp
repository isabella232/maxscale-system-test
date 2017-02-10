/**
 * @file mxs1110_16mb.cpp - trying to use LONGBLOB with > 16 mb data blocks
 * - try to insert large LONGBLOB via RWSplit in blocks > 16mb
 * - read data via RWsplit, ReadConn master, ReadConn slave, compare with inserted data
 */


#include "testconnections.h"
#include "blob_test.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(60);
    int chunk_size = 2500000;
    int chunk_num =5;

    Test->repl->execute_query_all_nodes( (char *) "set global max_allowed_packet=100000000");

    /*Test->connect_maxscale();
    Test->repl->connect();
    Test->tprintf("LONGBLOB: Trying send data directly to Master\n");
    test_longblob(Test, Test->repl->nodes[0], (char *) "LONGBLOB", 1000000, 20, 1);
    Test->repl->close_connections();
    Test->close_maxscale_connections();*/

    Test->connect_maxscale();
    Test->repl->connect();
    Test->tprintf("LONGBLOB: Trying send data via RWSplit\n");
    test_longblob(Test, Test->conn_rwsplit, (char *) "LONGBLOB", chunk_size, chunk_num, 2);
    Test->repl->close_connections();
    Test->close_maxscale_connections();

    Test->connect_maxscale();
    Test->tprintf("Checking data via RWSplit\n");
    check_longblob_data(Test, Test->conn_rwsplit, chunk_size, chunk_num, 2);
    Test->tprintf("Checking data via ReadConn master\n");
    check_longblob_data(Test, Test->conn_master, chunk_size, chunk_num, 2);
    Test->tprintf("Checking data via ReadConn slave\n");
    check_longblob_data(Test, Test->conn_slave, chunk_size, chunk_num, 2);
    Test->close_maxscale_connections();

    Test->copy_all_logs();
    return Test->global_result;
}

