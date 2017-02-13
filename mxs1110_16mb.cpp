/**
 * @file mxs1110_16mb.cpp - trying to use LONGBLOB with > 16 mb data blocks
 * - try to insert large LONGBLOB via RWSplit in blocks > 16mb
 * - read data via RWsplit, ReadConn master, ReadConn slave, compare with inserted data
 */


#include "testconnections.h"
#include "blob_test.h"
#include "fw_copy_rules.h"

int main(int argc, char *argv[])
{
    TestConnections::skip_maxscale_start(true);
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(60);
    int chunk_size = 2500000;
    int chunk_num =5;

    char str[1024];
    sprintf(str, "%s/masking/masking_user/masking_rules.json", Test->test_dir);
    Test->copy_to_maxscale(str, "~/");

    sprintf(str, "%s/cache/cache_basic/cache_rules.json", Test->test_dir);
    Test->copy_to_maxscale(str, "~/");

    char rules_dir[4096];
    sprintf(rules_dir, "%s/fw/", Test->test_dir);
    copy_rules(Test, (char *) "rules2", rules_dir);

    Test->start_maxscale();

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

    MYSQL * conn_galera = open_conn(4016, Test->maxscale_IP, Test->maxscale_user, Test->maxscale_password, Test->ssl);
    //check_longblob_data(Test, conn_galera, chunk_size, chunk_num, 2);
    mysql_close(conn_galera);

    Test->copy_all_logs();
    return Test->global_result;
}

