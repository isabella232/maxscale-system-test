/**
 * Test replication-manager
 */

#include "testconnections.h"

void get_output(TestConnections& test)
{
    test.tprintf("Maxadmin output:");
    char *output = test.ssh_maxscale_output(true, "maxadmin list servers");
    test.tprintf("%s", output);
    free(output);

    test.tprintf("replication-manager output:");
    output = test.ssh_maxscale_output(true, "cat /var/log/replication-manager.log && sudo truncate -s 0 /var/log/replication-manager.log");
    test.tprintf("%s", output);
    free(output);
}

void check(TestConnections& test)
{
    static int value = 0;
    MYSQL *conn = test.open_rwsplit_connection();
    test.try_query(conn, "INSERT INTO test.t1 VALUES (%d)", value++);
    mysql_close(conn);
}

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    test.tprintf("Installing replication-manager");
    int rc = system("./manage_mrm.sh install > manage_mrm.log");
    if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0)
    {
        test.tprintf("Failed to install replication-manager, see manage_mrm.log for more details");
        return -1;
    }

    // Wait a few seconds
    sleep(5);

    test.tprintf("Creating table and inserting data");
    test.connect_maxscale();
    test.try_query(test.conn_rwsplit, "CREATE OR REPLACE TABLE test.t1(id INT)");
    test.try_query(test.conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");

    check(test);
    get_output(test);

    test.tprintf("Stopping master and waiting for it to fail over");
    test.repl->stop_node(0);
    sleep(10);

    check(test);
    get_output(test);

    test.tprintf("Stopping another node and waiting for replication-manager to detect it");
    test.repl->stop_node(1);
    sleep(10);

    check(test);
    get_output(test);

    test.repl->start_node(0, (char*)"");
    test.repl->start_node(1, (char*)"");

    test.tprintf("Wait for replication-manager to fix the replication");
    sleep(10);

    check(test);
    get_output(test);

    test.tprintf("Dropping tables");
    test.close_maxscale_connections();
    test.connect_maxscale();
    test.try_query(test.conn_rwsplit, "DROP TABLE test.t1");
    test.close_maxscale_connections();

    get_output(test);

    system("./manage_mrm.sh remove >> manage_mrm.log");
    test.repl->fix_replication();
    return test.global_result;
}
