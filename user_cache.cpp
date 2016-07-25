/**
 * @file user_cache.cpp Test user caching mechanism of MaxScale
 *
 * - Create 'testuser'@'%' user
 * - Start up MaxScale with 'testuser' as the service user
 * - Delete 'testuser'@'%'
 * - Restart MaxScale
 * - Check that queries through MaxScale are OK
 */

#include <my_config.h>
#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->stop_timeout();
    Test->stop_maxscale();

    /** Create the test user and give required grants */
    Test->tprintf("Create the test user and give required grants\n");
    Test->repl->connect();
    execute_query_silent(Test->repl->nodes[0], "CREATE USER 'testuser'@'%' IDENTIFIED BY 'testpasswd'");
    execute_query_silent(Test->repl->nodes[0], "GRANT SELECT ON mysql.user TO 'testuser'@'%'");
    execute_query_silent(Test->repl->nodes[0], "GRANT SELECT ON mysql.db TO 'testuser'@'%'");
    execute_query_silent(Test->repl->nodes[0], "GRANT SELECT ON mysql.tables_priv TO 'testuser'@'%'");
    execute_query_silent(Test->repl->nodes[0], "GRANT SHOW DATABASES ON *.* TO 'testuser'@'%'");

    /** Wait for the user to replicate */
    Test->tprintf("Waiting for users to replicate\n");
    sleep(10);

    /** Test that MaxScale works and initialize the cache */
    Test->tprintf("Test that MaxScale works and initialize the cache\n");
    Test->start_maxscale();
    Test->connect_maxscale();
    Test->set_timeout(30);
    Test->add_result(Test->try_query_all("SHOW DATABASES"), "Initial query without user cache should work");
    Test->stop_timeout();

    /** Delete the service user */
    Test->tprintf("Deleting the service user\n");
    execute_query_silent(Test->repl->nodes[0], "CREATE USER 'testuser'@'%' IDENTIFIED BY 'testpasswd'");
    sleep(10);

    /** Restart MaxScale and check that the user cache works */
    Test->tprintf("Restart MaxScale and check that the user cache works\n");
    Test->restart_maxscale();
    Test->connect_maxscale();
    Test->set_timeout(30);
    Test->add_result(Test->try_query_all("SHOW DATABASES"), "Second query with user cache should work");
    Test->stop_timeout();

    Test->copy_all_logs();

    return Test->global_result;
}
