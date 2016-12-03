/**
 * MXS-922: Monitor creation test
 *
 */

#include "config_operations.h"

int main(int argc, char *argv[])
{
    TestConnections *test = new TestConnections(argc, argv);
    Config config(test);

    

    test->tprintf("Creating monitor");

    config.create_all_listeners();

    config.create_monitor("mysqlmon", 500);

    for (int i = 0; i < test->repl->N; i++)
    {
        config.create_server(i);
        config.add_server(i);
    }

    sleep(1);

    test->connect_maxscale();
    test->try_query(test->conn_rwsplit, "SELECT LAST_INSERT_ID()");

    config.reset();
    test->check_maxscale_alive();
    test->check_log_err("Fatal", false);
    test->copy_all_logs();
    return test->global_result;
}
