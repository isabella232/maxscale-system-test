#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

/**
 * Tests for the CCRFilter module
 */

static int master_id;

bool is_master(MYSQL *conn)
{
    char str[1024];

    if (find_field(conn, "SELECT @@server_id", "@@server_id", str) == 0)
    {
        return master_id == atoi(str);
    }

    return false;
}

int main(int argc, char *argv[])
{
    TestConnections * test = new TestConnections(argc, argv);

    test->repl->connect();

    /**
     * Get the master's @@server_id
     */
    master_id = test->repl->get_server_id(0);

    execute_query(test->repl->nodes[0], "CREATE OR REPLACE TABLE test.t1 (id INT);");
    execute_query(test->repl->nodes[0], "CREATE OR REPLACE TABLE test.t2 (id INT);");

    test->connect_maxscale();

    /**
     * Test `time`. The first SELECT within 10 seconds should go the
     * master and all SELECTs after it should go to the slaves.
     */
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    sleep(5);
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the first SELECT");
    sleep(7);
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the second SELECT");


    /**
     * Change test setup for `count`, the first three selects after an
     * insert should go to the master.
     */
    test->close_maxscale_connections();
    test->ssh_maxscale("sed -i -e 's/time/###time/' /etc/maxscale.cnf");
    test->ssh_maxscale("sed -i -e 's/###count/count/' /etc/maxscale.cnf");
    test->restart_maxscale();
    test->connect_maxscale();

    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the first SELECT");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the second SELECT");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the third SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the fourth SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the fifth SELECT");

    /**
     * Change test setup for `count` and `match`, selects after an insert
     * to t1 should go to the slaves and selects after an insert to t2
     * should go to the master.
     */
    test->close_maxscale_connections();
    test->ssh_maxscale("sed -i -e 's/###match/match/' /etc/maxscale.cnf");
    test->restart_maxscale();
    test->connect_maxscale();


    /** t1 first, should be ignored */
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the first SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the second SELECT");

    /** t2 should match and trigger the critical reads */
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t2 VALUES (1)");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the first SELECT");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the second SELECT");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the third SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the fourth SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the fifth SELECT");

    /**
     * Change test setup for `count` and `ignore`, expects the same
     * results as previous test.
     */
    test->close_maxscale_connections();
    test->ssh_maxscale("sed -i -e 's/match/###match/' /etc/maxscale.cnf");
    test->ssh_maxscale("sed -i -e '/###ignore/ignore/' /etc/maxscale.cnf");
    test->restart_maxscale();
    test->connect_maxscale();

    /** t1 first, should be ignored */
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the first SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the second SELECT");

    /** t2 should match and trigger the critical reads */
    test->try_query(test->conn_rwsplit, "INSERT INTO test.t2 VALUES (1)");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the first SELECT");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the second SELECT");
    test->add_result(is_master(test->conn_rwsplit), "Master should reply to the third SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the fourth SELECT");
    test->add_result(!is_master(test->conn_rwsplit), "Master should NOT reply to the fifth SELECT");

    execute_query(test->repl->nodes[0], "DROP TABLE test.t1");
    execute_query(test->repl->nodes[0], "DROP TABLE test.t2");

    return test->global_result;
}
