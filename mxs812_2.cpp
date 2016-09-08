/**
 * @file binary_ps.cpp - Execute binary protocol prepared statements while master is blocked
 */

#include <my_config.h>
#include "testconnections.h"

int test_ps(TestConnections* Test, MYSQL * conn)
{
    const char select_stmt[] = "SELECT ?, ?, ?, ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);

    mysql_stmt_prepare(stmt, select_stmt, sizeof(select_stmt) - 1);

    int value = 1;
    MYSQL_BIND param[4];

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].is_null = 0;
    param[0].buffer = &value;
    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].is_null = 0;
    param[1].buffer = &value;
    param[2].buffer_type = MYSQL_TYPE_LONG;
    param[2].is_null = 0;
    param[2].buffer = &value;
    param[3].buffer_type = MYSQL_TYPE_LONG;
    param[3].is_null = 0;
    param[3].buffer = &value;

    mysql_stmt_bind_param(stmt, param);
    mysql_stmt_execute(stmt);
    mysql_stmt_close(stmt);

    return 0;
}

static bool running = true;

void* test_thr(void *data)
{
    TestConnections *Test = (TestConnections*)data;

    while (running)
    {
        MYSQL *mysql = Test->open_rwsplit_connection();

        for (int i = 0; i < 3; i++)
        {
            test_ps(Test, mysql);
        }

        mysql_close(mysql);
    }
}

#define THREADS 5

int main(int argc, char *argv[])
{
    TestConnections *Test = new TestConnections(argc, argv);
    pthread_t thr[THREADS];
    int iter = 5;

    for (int i = 0; i < THREADS; i++)
    {
        pthread_create(&thr[i], NULL, test_thr, Test);
    }

    for (int i = 0; i < iter; i++)
    {
        sleep(5);
        Test->repl->block_node(0);
        sleep(5);
        Test->repl->unblock_node(0);
    }

    running = false;

    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(thr[i], NULL);
    }

    Test->check_maxscale_alive();

    char *output = Test->ssh_maxscale_output(true, "maxadmin show servers|grep -i 'current.*operations'|sed 's/.*://'|sed 's/[^0-9]//g'|sort -r|uniq");
    char *nl = strchr(output, '\n');
    *nl = '\0';
    Test->tprintf("Number of operations: %s", output);

    Test->add_result(strcmp(output, "0"), "Expected 0 active operations on all servers, got %s.", output);

    Test->copy_all_logs();

    return Test->global_result;
}
