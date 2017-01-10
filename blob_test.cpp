#include "blob_test.h"

int test_longblob(TestConnections* Test, MYSQL * conn, char * blob_name, unsigned long chunk_size, int chunks, int rows)
{
    unsigned long size = chunk_size;
    unsigned long * data;
    unsigned long i;
    MYSQL_BIND param[1];
    char sql[256];
    int global_res = Test->global_result;
    //Test->tprintf("chunk size %lu chunks %d inserts %d\n", chunk_size, chunks, rows);

    char *insert_stmt = (char *) "INSERT INTO long_blob_table(x, b) VALUES(1, ?)";

    Test->tprintf("Creating table with %s\n", blob_name);
    Test->try_query(conn, (char *) "DROP TABLE IF EXISTS long_blob_table");
    sprintf(sql, "CREATE TABLE long_blob_table(id int NOT NULL AUTO_INCREMENT, x INT, b %s, PRIMARY KEY (id))", blob_name);
    Test->try_query(conn, sql);

    Test->tprintf("Preparintg INSERT stmt\n");
    MYSQL_STMT * stmt = mysql_stmt_init(conn);
    if (stmt == NULL)
    {
        Test->add_result(1, "stmt init error: %s\n", mysql_error(conn));
    }

    Test->add_result(mysql_stmt_prepare(stmt, insert_stmt, strlen(insert_stmt)), "Error preparing stmt: %s\n", mysql_stmt_error(stmt));

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

    Test->tprintf("Sending data in %d bytes chunks, total size is %d\n", size * sizeof(unsigned long), (size * sizeof(unsigned long)) * chunks);
    for (i = 0; i < chunks; i++) {
        Test->set_timeout(300);
        Test->tprintf("Chunk #%d\n", i);
        if (mysql_stmt_send_long_data(stmt, 0, (char *) data, size * sizeof(unsigned long)) != 0) {
            Test->add_result(1, "Error inserting data, iteration %d, error %s\n", i, mysql_stmt_error(stmt));
            return 1;
        }
    }

    for (int k = 0; k < rows; k++)
    {
        Test->tprintf("Executing statement: %02d\n", k);
        Test->set_timeout(3000);
        Test->add_result(mysql_stmt_execute(stmt), "INSERT Statement with %s failed, error is %s\n", blob_name, mysql_stmt_error(stmt));
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

