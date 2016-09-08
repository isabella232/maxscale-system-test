/**
 * @file longblob.cpp - trying to use LONGBLOB
 */

#include <my_config.h>
#include "testconnections.h"

int test_longblob(TestConnections* Test, MYSQL * conn, char * blob_name, long int chunk_size, int chunks, bool with_sescmd)
{
    unsigned long size = chunk_size;
    unsigned long * data;
    unsigned long i;
    MYSQL_BIND param[1];
    char sql[256];
    int global_res = Test->global_result;

    char *insert_stmt = (char *) "INSERT INTO long_blob_table(x, b) VALUES(1, ?)";

    Test->tprintf("Creating table with %s\n", blob_name);
    Test->try_query(conn, (char *) "DROP TABLE IF EXISTS long_blob_table");
    sprintf(sql, "CREATE TABLE long_blob_table(x INT, b %s)", blob_name);
    Test->try_query(conn, sql);

    Test->tprintf("Preparintg INSERT stmt\n");
    MYSQL_STMT * stmt = mysql_stmt_init(conn);
    if (stmt == NULL)
    {
        Test->add_result(1, "stmt init error: %s\n", mysql_error(conn));
    }

    Test->add_result(mysql_stmt_prepare(stmt, insert_stmt, strlen(insert_stmt)), "Error preparing stmt: %s\n", mysql_stmt_error(stmt));

    if (with_sescmd)
    {
        Test->try_query(conn, (char *) "USE test");
    }

    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].is_null = 0;

    Test->tprintf("Binding parameter\n");
    Test->add_result(mysql_stmt_bind_param(stmt, param), "Error parameter binding: %s\n", mysql_stmt_error(stmt));

    Test->tprintf("Filling buffer\n");
    data = (unsigned long *) malloc(size * sizeof(long int));

    if (data == NULL)
    {
        Test->add_result(1, "Memory allocation error\n");
    }

    for (i = 0; i < size; i++)
    {
        data[i] = i;
    }

    Test->tprintf("Sending data in %d bytes chunks, total size is %d\n", size * sizeof(long int), (size * sizeof(long int)) * chunks);
    for (i = 0; i < chunks; i++) {
        Test->set_timeout(3000);
        Test->tprintf("Chunk #%d\n", i);
        if (mysql_stmt_send_long_data(stmt, 0, (char *) data, size * sizeof(long int)) != 0) {
            Test->add_result(1, "Error inserting data, iteration %d, error %s\n", i, mysql_stmt_error(stmt));
            return 1;
        }
    }
    Test->tprintf("Executing statement\n");
    Test->set_timeout(3000);
    Test->add_result(mysql_stmt_execute(stmt), "INSERT Statement with %s failed, error is %s\n", blob_name, mysql_stmt_error(stmt));

    if (with_sescmd)
    {
        Test->try_query(conn, (char *) "USE test");
    }

    Test->add_result(mysql_stmt_close(stmt), "Error closing stmt\n");

    if (global_res == Test->global_result)
    {
        Test->tprintf("%s is OK\n", blob_name);
    } else {
        Test->tprintf("%s FAILED\n", blob_name);
    }

    return 0;
}

void run_test(TestConnections *Test, const char *type, size_t size, int chunks, bool sescmd)
{
    Test->connect_maxscale(); Test->repl->connect();
    Test->tprintf("LONGBLOB: Trying send data via RWSplit\n");
    test_longblob(Test, Test->conn_rwsplit, (char *) type, size, chunks, sescmd);
    Test->repl->close_connections(); Test->close_maxscale_connections();
}


int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(600);

    Test->repl->execute_query_all_nodes( (char *) "set global max_allowed_packet=10000000");

    for (int i = 0; i < 500; i++)
        run_test(Test, "BLOB", 500000, 10, true);

    Test->copy_all_logs(); return(Test->global_result);
}
