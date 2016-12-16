/**
 * MXS-922: Server creation test
 *
 */

#include "testconnections.h"
#include "config_operations.h"

int main(int argc, char *argv[])
{
    TestConnections *test = new TestConnections(argc, argv);
    Config config(test);

    config.create_all_listeners();
    config.create_monitor("mysql-monitor", "mysqlmon", 500);

    test->tprintf("Testing server creation and destruction");

    config.create_server(1);
    config.create_server(1);
    config.check_server_count(1);
    config.destroy_server(1);
    config.destroy_server(1);
    config.check_server_count(0);
    test->check_maxscale_processes(1);

    test->tprintf("Testing adding of server to service");

    config.create_server(1);
    config.add_server(1);
    config.check_server_count(1);
    sleep(1);
    test->check_maxscale_alive();
    config.remove_server(1);
    config.destroy_server(1);
    config.check_server_count(0);

    test->tprintf("Testing altering of server");

    config.create_server(1);
    config.add_server(1);
    config.alter_server(1, "address", test->repl->IP[1]);
    sleep(1);
    test->check_maxscale_alive();
    config.remove_server(1);
    config.destroy_server(1);

    config.reset();
    sleep(1);
    test->check_maxscale_alive();
    test->copy_all_logs();
    return test->global_result;
}
