/**
 * @file mxs37_table_privilege.cpp mxs37 (bug719) regression case ("mandatory SELECT privilege on db level?")
 *
 */



#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include "sql_t1.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(60);

    Test->connect_maxscale();

    Test->tprintf("Create user with only SELECT priviledge to a table");

    execute_query_silent(Test->conn_rwsplit, "DROP USER 'table_privilege'@'%'");
    execute_query_silent(Test->conn_rwsplit, "DROP TABLE test.t1");
    execute_query(Test->conn_rwsplit, "CREATE TABLE test.t1 (id INT)");
    execute_query(Test->conn_rwsplit, "CREATE USER 'table_privilege'@'%%' IDENTIFIED BY 'pass'");
    execute_query(Test->conn_rwsplit, "GRANT SELECT ON test.t1 TO 'table_privilege'@'%%'");

    Test->stop_timeout();
    Test->repl->sync_slaves();

    Test->tprintf("Trying to connect using this user\n");
    Test->set_timeout(20);

    MYSQL *conn = open_conn_db(Test->rwsplit_port, Test->maxscale_IP, (char *) "test",
                               (char *) "table_privilege", (char *) "pass", Test->ssl);
    Test->add_result(mysql_errno(conn) != 0, "Failed to connect: %s", mysql_error(conn));

    Test->set_timeout(20);
    Test->tprintf("Trying SELECT\n");
    Test->try_query(conn, (char *) "SELECT * FROM t1");
    mysql_close(conn);

    Test->set_timeout(20);
    execute_query_silent(Test->conn_rwsplit, "DROP USER 'table_privilege'@'%'");
    execute_query_silent(Test->conn_rwsplit, "DROP TABLE test.t1");

    Test->check_maxscale_alive();
    Test->copy_all_logs();

    return Test->global_result;
}

