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
    config.create_monitor("mysql-monitor", "mysqlmon", 500);
    config.reset();

    sleep(1);

    test->check_maxscale_alive();

    config.destroy_monitor("mysql-monitor");

    test->check_maxscale_alive();

    test->ssh_maxscale_sh(true, "for i in 0 1 2 3; do maxadmin clear server server$i running; done");

    test->add_result(test->connect_maxscale() == 0, "Should not be able to connect");

    config.create_monitor("mysql-monitor2", "mysqlmon", 500);
    config.add_created_servers("mysql-monitor2");

    sleep(1);

    test->check_maxscale_alive();

    test->check_log_err("Fatal", false);
    test->copy_all_logs();
    return test->global_result;
}
