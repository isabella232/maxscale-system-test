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
    config.reset();

    sleep(1);

    test->check_maxscale_alive();
    test->check_log_err("Fatal", false);
    test->copy_all_logs();
    return test->global_result;
}
