/**
 * @file longblob.cpp - trying to use LONGBLOB
 */

#include <my_config.h>
#include "testconnections.h"

int test_ps(TestConnections* Test, MYSQL * conn)
{
    MYSQL_BIND param[4];
    int global_res = Test->global_result;
    char *insert_stmt = (char *) "SELECT ?, ?, ?, ?";
    MYSQL_STMT * stmt = mysql_stmt_init(conn);

    if (stmt == NULL)
    {
        Test->add_result(1, "stmt init error: %s\n", mysql_error(conn));
    }

    Test->add_result(mysql_stmt_prepare(stmt, insert_stmt, strlen(insert_stmt)), "Error preparing stmt: %s\n", mysql_stmt_error(stmt));

    int value = 1;

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

    Test->add_result(mysql_stmt_bind_param(stmt, param), "Error parameter binding: %s\n", mysql_stmt_error(stmt));
    Test->add_result(mysql_stmt_execute(stmt), "Statement failed, error is %s\n", mysql_stmt_error(stmt));
    Test->add_result(mysql_stmt_close(stmt), "Error closing stmt\n");

    return 0;
}

static bool running = true;

void* test_thr(void *data)
{
    TestConnections *Test = (TestConnections*)data;

    while (running)
    {
        MYSQL *mysql = Test->open_rwsplit_connection();
        test_ps(Test, mysql);
        mysql_close(mysql);
    }

}

#define THREADS 5

int main(int argc, char *argv[])
{
    TestConnections *Test = new TestConnections(argc, argv);
    pthread_t thr[THREADS];

    for (int i = 0; i < THREADS; i++)
    {
        pthread_create(&thr[i], NULL, test_thr, Test);
    }

    sleep(5);
    Test->repl->block_node(3);
    sleep(5);
    Test->repl->block_node(2);
    sleep(5);
    Test->repl->block_node(1);
    sleep(5);
    Test->repl->block_node(0);
    sleep(5);

    running = false;

    for (int i = 0; i < THREADS; i++)
    {
        pthread_join(thr[i], NULL);
    }

    Test->copy_all_logs(); return(Test->global_result);
}
