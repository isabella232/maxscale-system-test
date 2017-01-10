#include <iostream>
#include "testconnections.h"
#include "sql_t1.h"
#include "mariadb_func.h"
#include "blob_test.h"

using namespace std;

const char * test03_sql =
        " CREATE PROCEDURE multi()\n"
        "BEGIN\n"
        "SELECT x1 FROM t1 LIMIT 2;\n"
        "END";

const char * test04_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
            "SELECT 1;\n"
            "SELECT x1 FROM t1 LIMIT 2;\n"
            "SELECT 1,2,3; \n"
        "END";

const char * test05_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
            "SELECT 1;\n"
            "SELECT x1 FROM t1 LIMIT 8;\n"
            "SELECT 1,2,3; \n"
            "SELECT 1;"
        "END";

const char * test06_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
            "SELECT 1;\n"
            "SELECT x1 FROM t1 LIMIT 18;\n"
            "SELECT 2; \n"
            "SELECT 2;"
        "END";

const char * test07_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
         "SELECT 1,2,3,4;\n"
         "SELECT id, b from long_blob_table order by id desc limit 1;\n"
         "SELECT id, b from long_blob_table order by id desc limit 4;\n"
         "SELECT id, b from long_blob_table order by id desc limit 1;\n"
         "SELECT id, b from long_blob_table order by id desc limit 10;\n"
         "SELECT 1;\n"
        "END";

const char * test08_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
         "SELECT 1,2,3;\n"
         "SELECT id, b, b from long_blob_table order by id desc limit 1;\n"
         "SELECT 2;\n"
         "SELECT id, b from long_blob_table order by id desc limit 6;\n"
         "SELECT 1;\n"
         "SELECT 1;\n"
         "SELECT x1 FROM t1 LIMIT 8;\n"
         "SELECT 1;\n"
         "SELECT 1,2,3,4;\n"
        "END";

const char * test10_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
         "SELECT 1;\n"
         "SELECT x1 FROM t1 limit 4;\n"
         "select * from dual;\n"
         "set @a=4;\n"
         "SELECT 2;\n"
         "SELECT * FROM t1;\n"
        "END";

const char * test14_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
         "SELECT 1,3;\n"
         "SET @table = 't1';\n"
         "SET @s = CONCAT('SELECT * FROM ', @table, ' LIMIT 18');\n"
         "PREPARE stmt1 FROM @s;\n"
         "EXECUTE stmt1;\n"
         "DEALLOCATE PREPARE stmt1;\n"
         "SELECT 2,4,5;\n"
        "END";

const char * test15_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
         "SELECT 1,3;\n"
         "SET @table = 't1';\n"
         "SET @s = CONCAT('SELECT * FROM ', @table, ' LIMIT 100');\n"
         "PREPARE stmt1 FROM @s;\n"
         "EXECUTE stmt1;\n"
         "DEALLOCATE PREPARE stmt1;\n"
         "SELECT 2,4,5;\n"
        "END";

const char * test17_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
            "SELECT '' as 'A' limit 1;\n"
            "SELECT '' as 'A' limit 10;\n"
            "SELECT '' as 'A';\n"
        "END";

const char * test18_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
            "SELECT '' as 'A' limit 1;\n"
            "SELECT '' as 'A' limit 10;\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A' limit 1;\n"
            "SELECT '' as 'A' limit 10;\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
        "END";

const char * test19_sql =
        "CREATE PROCEDURE multi() BEGIN\n"
            "SELECT '' as 'A' limit 1;\n"
            "SELECT '' as 'A' limit 10;\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A' limit 1;\n"
            "SELECT '' as 'A' limit 10;\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
            "SELECT '' as 'A';\n"
        "END";


int compare_expected(TestConnections * Test, const char * sql, my_ulonglong exp_i, my_ulonglong exp_rows[])
{
    my_ulonglong *rows = new my_ulonglong[30];
    my_ulonglong i;

    Test->set_timeout(10);
    execute_query_num_of_rows(Test->conn_rwsplit, sql, rows, &i);

    Test->tprintf("Result sets number is %llu\n", i);

    if ( i != exp_i)
    {
        Test->add_result(1, "Number of result sets is %llu instead of %llu\n", i, exp_i);
        return 1;
    }

    for (my_ulonglong j = 0; j < i; j++)
    {
        Test->tprintf("For result set %llu number of rows is %llu\n", j, rows[j]);
        if (rows[j] != exp_rows[j])
        {
            Test->add_result(1, "For result set %llu number of rows is %llu instead of %llu\n", j, rows[j], exp_rows[j]);
            return 1;
        }
    }
    return 0;
}

int compare_stmt_expected(TestConnections * Test, MYSQL_STMT * stmt, my_ulonglong exp_i, my_ulonglong exp_rows[])
{
    my_ulonglong *rows = new my_ulonglong[30];
    my_ulonglong i;

    Test->set_timeout(10);
    execute_stmt_num_of_rows(stmt, rows, &i);

    Test->tprintf("Result sets number is %llu\n", i);

    if ( i != exp_i)
    {
        Test->add_result(1, "Number of result sets is %llu instead of %llu\n", i, exp_i);
        return 1;
    }

    for (my_ulonglong j = 0; j < i; j++)
    {
        Test->tprintf("For result set %llu number of rows is %llu\n", j, rows[j]);
        if (rows[j] != exp_rows[j])
        {
            Test->add_result(1, "For result set %llu number of rows is %llu instead of %llu\n", j, rows[j], exp_rows[j]);
            return 1;
        }
    }
    return 0;
}


void err_check(TestConnections* Test, unsigned int expected_err)
{
    Test->tprintf("Error text %s error code %d\n", mysql_error(Test->conn_rwsplit), mysql_errno(Test->conn_rwsplit));
    if (mysql_errno(Test->conn_rwsplit) != expected_err)
    {
        Test->add_result(1, "Error code is not %d, it is %d\n", expected_err, mysql_errno(Test->conn_rwsplit));
    }
}

int main(int argc, char *argv[])
{

    my_ulonglong *exp_rows = new my_ulonglong[30];
    MYSQL_STMT * stmt;

    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(10);
    Test->connect_rwsplit();

    create_t1(Test->conn_rwsplit);
    insert_into_t1(Test->conn_rwsplit, 1);
    Test->stop_timeout();
    sleep(5);


    Test->tprintf("**** Test 1 ****\n");


    exp_rows[0] = 16;
    compare_expected(Test, (char *) "select * from t1", 1, exp_rows);

    exp_rows[0] = 16;
    compare_expected(Test, (char *) "select * from t1 where fl=0", 1, exp_rows);

    exp_rows[0] = 10;
    compare_expected(Test, (char *) "select * from t1 limit 10", 1, exp_rows);

    Test->set_timeout(60);
    create_t1(Test->conn_rwsplit);
    insert_into_t1(Test->conn_rwsplit, 3);
    Test->stop_timeout();
    sleep(5);

    Test->tprintf("**** Test 2 ****\n");
    exp_rows[0] = 0;
    compare_expected(Test, (char *) "select * from t1", 1, exp_rows);

    exp_rows[0] = 16;
    compare_expected(Test, (char *) "select * from t1 where fl=0", 1, exp_rows);

    exp_rows[0] = 10;
    compare_expected(Test, (char *) "select * from t1 limit 10", 1, exp_rows);

    Test->tprintf("**** Test 3 ****\n");
    exp_rows[0] = 2;
    exp_rows[1] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test03_sql);
    compare_expected(Test, "CALL multi()", 2, exp_rows);

    Test->tprintf("**** Test 4 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 2;
    exp_rows[2] = 1;
    exp_rows[3] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test04_sql);
    compare_expected(Test, "CALL multi()", 4, exp_rows);

    Test->tprintf("**** Test 5 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 8;
    exp_rows[2] = 1;
    exp_rows[3] = 1;
    exp_rows[4] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test05_sql);
    compare_expected(Test, "CALL multi()", 5, exp_rows);

    Test->tprintf("**** Test 6 ****\n");
    exp_rows[0] = 0;

    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test06_sql);
    compare_expected(Test, "CALL multi()", 1, exp_rows);


    Test->tprintf("LONGBLOB: Trying send data via RWSplit\n");
    Test->try_query(Test->conn_rwsplit, "SET GLOBAL max_allowed_packet=10000000000");
    Test->stop_timeout();
    Test->repl->connect();
    //test_longblob(Test, Test->conn_rwsplit, (char *) "LONGBLOB", 512 * 1024 / sizeof(long int), 17 * 2, 25);
    test_longblob(Test, Test->repl->nodes[0], (char *) "LONGBLOB", 512 * 1024 / sizeof(long int), 17 * 2, 25);
    Test->repl->close_connections();


    Test->tprintf("**** Test 7 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 1;
    exp_rows[2] = 4;
    exp_rows[3] = 1;
    exp_rows[4] = 10;
    exp_rows[5] = 1;
    exp_rows[6] = 0;


    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test07_sql);
    compare_expected(Test, "CALL multi()", 7, exp_rows);

    Test->tprintf("**** Test 8 ****\n");
    exp_rows[0] = 0;

    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test08_sql);
    compare_expected(Test, "CALL multi()", 1, exp_rows);

    Test->tprintf("**** Test 9 ****\n");
    exp_rows[0] = 0;

    compare_expected(Test, "SELECT * FROM dual", 0, exp_rows);
    err_check(Test, 1096);

    Test->tprintf("**** Test 10 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 4;

    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test10_sql);
    compare_expected(Test, "CALL multi()", 2, exp_rows);

    err_check(Test, 1096);

    Test->tprintf("**** Test 11 ****\n");
    exp_rows[0] = 0;

    compare_expected(Test, "SET @a=4;", 1, exp_rows);
    err_check(Test, 0);

    // Prepared statements

    /* Temporary disabled
    Test->tprintf("**** Test 12 (C ) ****\n");
    exp_rows[0] = 0;

    stmt = mysql_stmt_init(Test->conn_rwsplit);
    if (stmt == NULL)
    {
        Test->add_result(1, "stmt init error: %s\n", mysql_stmt_error(stmt));
    }
    char *stmt1 = (char *) "SELECT * FROM t1";
    Test->add_result(mysql_stmt_prepare(stmt, stmt1, strlen(stmt1)), "Error preparing stmt: %s\n", mysql_stmt_error(stmt));

    compare_stmt_expected(Test, stmt, 1, exp_rows);

    mysql_stmt_close(stmt); */



    Test->tprintf("**** Test 12 (MariaDB command line client) ****\n");
    exp_rows[0] = 0;
    Test->try_query(Test->conn_rwsplit, "SET @table = 't1'");
    Test->try_query(Test->conn_rwsplit, "SET @s = CONCAT('SELECT * FROM ', @table)");
    Test->try_query(Test->conn_rwsplit, "PREPARE stmt1 FROM @s");
    compare_expected(Test, "EXECUTE stmt1", 1, exp_rows);
    Test->try_query(Test->conn_rwsplit, "DEALLOCATE PREPARE stmt1");


    Test->tprintf("**** Test 13 (C )****\n");
    exp_rows[0] = 10;
    exp_rows[1] = 0;
    stmt = mysql_stmt_init(Test->conn_rwsplit);
    if (stmt == NULL)
    {
        Test->add_result(1, "stmt init error: %s\n", mysql_stmt_error(stmt));
    }
    char *stmt2 = (char *) "SELECT * FROM t1 LIMIT 10";
    Test->add_result(mysql_stmt_prepare(stmt, stmt2, strlen(stmt2)), "Error preparing stmt: %s\n", mysql_stmt_error(stmt));
    compare_stmt_expected(Test, stmt, 1, exp_rows);
    mysql_stmt_close(stmt);

    Test->tprintf("**** Test 13 (MariaDB command line client) ****\n");
    Test->try_query(Test->conn_rwsplit, "SET @table = 't1'");
    Test->try_query(Test->conn_rwsplit, "SET @s = CONCAT('SELECT * FROM ', @table,  ' LIMIT 10')");
    Test->try_query(Test->conn_rwsplit, "PREPARE stmt1 FROM @s");
    compare_expected(Test, "EXECUTE stmt1", 1, exp_rows);
    Test->try_query(Test->conn_rwsplit, "DEALLOCATE PREPARE stmt1");

    Test->tprintf("**** Test 14 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 18;
    exp_rows[2] = 1;
    exp_rows[3] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test14_sql);
    compare_expected(Test, "CALL multi()", 4, exp_rows);

    Test->tprintf("**** Test 15 ****\n");
    exp_rows[0] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test15_sql);
    compare_expected(Test, "CALL multi()", 1, exp_rows);

    Test->tprintf("**** Test 16 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 0;
    compare_expected(Test, "SELECT '' as 'A' limit 1;", 1, exp_rows);

    Test->tprintf("**** Test 17 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 1;
    exp_rows[2] = 1;
    exp_rows[3] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test17_sql);
    compare_expected(Test, "CALL multi()", 4, exp_rows);

    Test->tprintf("**** Test 18 ****\n");
    exp_rows[0] = 1;
    exp_rows[1] = 1;
    exp_rows[2] = 1;
    exp_rows[3] = 1;
    exp_rows[4] = 1;
    exp_rows[5] = 1;
    exp_rows[6] = 1;
    exp_rows[7] = 1;
    exp_rows[8] = 1;
    exp_rows[9] = 1;
    exp_rows[10] = 1;
    exp_rows[11] = 1;
    exp_rows[12] = 1;
    exp_rows[13] = 1;
    exp_rows[14] = 1;
    exp_rows[15] = 1;
    exp_rows[16] = 1;
    exp_rows[17] = 1;
    exp_rows[18] = 1;
    exp_rows[19] = 1;
    exp_rows[20] = 0;
    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test18_sql);
    compare_expected(Test, "CALL multi()", 21, exp_rows);

    Test->tprintf("**** Test 19 ****\n");
    exp_rows[0] = 0;

    Test->try_query(Test->conn_rwsplit, "DROP PROCEDURE IF EXISTS multi");
    Test->try_query(Test->conn_rwsplit, test19_sql);
    compare_expected(Test, "CALL multi()", 1, exp_rows);

    Test->tprintf("**** Test 20 ****\n");
    exp_rows[0] = 2;
    exp_rows[1] = 0;
    Test->try_query(Test->conn_rwsplit, "SET GLOBAL max_allowed_packet=10000000000");
    compare_expected(Test, "SELECT * FROM long_blob_table limit 2;", 1, exp_rows);
    err_check(Test, 0);

    Test->close_rwsplit();

    Test->ssh_maxscale(true, "sed -i \"s/max_resultset_size=900000/max_resultset_size=90/\" /etc/maxscale.cnf");
    Test->set_timeout(100);
    Test->restart_maxscale();

    Test->connect_rwsplit();

    Test->tprintf("**** Test 21 ****\n");
    exp_rows[0] = 0;
    Test->try_query(Test->conn_rwsplit, "SET GLOBAL max_allowed_packet=10000000000");
    compare_expected(Test, "SELECT * FROM long_blob_table limit 1;", 1, exp_rows);

    Test->check_maxscale_alive();
    Test->copy_all_logs();

    return Test->global_result;

}
