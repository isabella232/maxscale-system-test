/**
 * @file bug676.cpp  reproducing attempt for bug676
 * - connect to RWSplit
 * - stop node0
 * - sleep 30 seconds
 * - reconnect
 * - check if 'USE test ' is ok
 * - check MaxScale is alive
 */

#include <iostream>
#include "testconnections.h"
#include "mariadb_func.h"

int main(int argc, char *argv[])
{
    TestConnections test(argc, argv);
    test.set_timeout(20);

    MYSQL * conn = open_conn_no_db(test.rwsplit_port, test.maxscale_IP, test.maxscale_user,
                                   test.maxscale_password, test.ssl);

    test.tprintf("Stopping %d\n", 0);
    test.galera->block_node(0);

    test.stop_timeout();
    sleep(30);
    test.set_timeout(20);
    mysql_close(conn);

    conn = open_conn_no_db(test.rwsplit_port, test.maxscale_IP, test.maxscale_user, test.maxscale_password,
                           test.ssl);

    test.add_result(conn == 0, "Error connection to RW Split\n");

    test.tprintf("selecting DB 'test' for rwsplit\n");
    test.try_query(conn, "USE test");

    test.tprintf("Closing connection\n");
    mysql_close(conn);

    test.connect_rwsplit();
    test.try_query(test.conn_rwsplit, "show processlist;");
    test.close_maxscale_connections();

    test.stop_timeout();
    test.galera->unblock_node(0);

    return test.global_result;
}

