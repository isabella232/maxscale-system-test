/**
 * MXS-922: Server scaling test
 *
 */

#include "testconnections.h"

#define MONITOR_NAME "mysql-monitor"
#define SERVICE_NAME "rwsplit-service"

static bool running = true;

void* query_thread(void *data)
{
    TestConnections *test = static_cast<TestConnections*>(data);

    MYSQL *mysql = test->open_rwsplit_connection();
    my_bool yes = true;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &yes);

    while (running)
    {
        execute_query_silent(mysql, "SELECT @@server_id");
        execute_query_silent(mysql, "SELECT last_insert_id()");
    }

    mysql_close(mysql);

    return NULL;
}

void add_server(TestConnections *test, int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin add server server%d " MONITOR_NAME, num);
    test->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME, num);
    test->stop_timeout();
}

void remove_server(TestConnections *test, int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin remove server server%d " MONITOR_NAME, num);
    test->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME, num);
    test->stop_timeout();
}

void destroy_server(TestConnections *test, int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin destroy server server%d", num);
    test->stop_timeout();
}

void create_server(TestConnections *test, int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin create server server%d %s",
                       num, test->repl->IP[num]);
    test->stop_timeout();
}

int main(int argc, char *argv[])
{
    TestConnections *test = new TestConnections(argc, argv);
    int num_threads = 5;
    int iterations = test->smoke ? 10 : 50;
    pthread_t threads[num_threads];

    test->tprintf("Creating client threads");

    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, query_thread, test);
    }


    test->tprintf("Adding and removing servers for %d seconds.", iterations * test->repl->N);

    for (int x = 0; x < iterations; x++)
    {
        for (int i = 0; i < test->repl->N; i++)
        {
            if ((x + i) % 2 == 0)
            {
                create_server(test, i);
                add_server(test, i);
                remove_server(test, i);
                destroy_server(test,i);
            }
            else
            {
                remove_server(test, i);
                destroy_server(test,i);
                create_server(test, i);
                add_server(test, i);
            }

            sleep(1);
        }
    }

    running = false;

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    /** Make sure the servers exist before checking that connectivity is OK */
    for (int i = 0; i < test->repl->N; i++)
    {
        create_server(test, i);
        add_server(test, i);
    }

    test->check_maxscale_alive();
    test->check_log_err("Fatal", false);
    test->copy_all_logs();
    return test->global_result;
}
