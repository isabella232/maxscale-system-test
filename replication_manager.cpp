/**
 * Test replication-manager
 */

#include "testconnections.h"

int main(int argc, char** argv)
{
    TestConnections test(argc, argv); 
    test.tprintf("Installing replication-manager");
    system("./manage_mrm.sh install");

    test.tprintf("Creating table and inserting data");
    test.connect_maxscale();
    test.try_query(test.conn_rwsplit, "CREATE OR REPLACE TABLE test.t1(id INT)");
    test.try_query(test.conn_rwsplit, "INSERT INTO test.t1 VALUES (1)");
    test.close_maxscale_connections();

    test.tprintf("Maxadmin output before failover:");
    char *output = test.ssh_maxscale_output(true, "maxadmin list servers");
    test.tprintf("%s", output);
    free(output);

    test.tprintf("replication-manager output before failover:");
    output = test.ssh_maxscale_output(true, "cat /var/log/replication-manager.log && sudo truncate -s 0 /var/log/replication-manager.log");
    test.tprintf("%s", output);
    free(output);

    test.tprintf("Stopping master and waiting for it to fail over");
    test.repl->stop_node(0);
    sleep(60);

    test.tprintf("Maxadmin output after failover:");
    output = test.ssh_maxscale_output(true, "maxadmin list servers");
    test.tprintf("%s", output);
    free(output);

    test.tprintf("replication-manager output after failover:");
    output = test.ssh_maxscale_output(true, "cat /var/log/replication-manager.log");
    test.tprintf("%s", output);
    free(output);

    test.tprintf("Inserting more data and dropping tables");
    test.connect_maxscale();
    test.try_query(test.conn_rwsplit, "INSERT INTO test.t1 VALUES (2)");
    test.try_query(test.conn_rwsplit, "DROP TABLE test.t1");
    test.close_maxscale_connections();

    system("./manage_mrm.sh remove");
    test.repl->fix_replication();
    return test.global_result;
}
