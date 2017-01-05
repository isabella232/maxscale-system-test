#include <iostream>
#include "testconnections.h"
#include "sql_t1.h"
#include "mariadb_func.h"

using namespace std;

int compare_expected(TestConnections * Test, char * sql, my_ulonglong exp_i, my_ulonglong exp_rows[])
{
    my_ulonglong *rows = new my_ulonglong[10];
    my_ulonglong i;

    Test->set_timeout(10);
    execute_query_num_of_rows(Test->conn_rwsplit, sql, rows, &i);

    Test->tprintf("Result sets number is %llu, first result set contains %llu rows\n", i, rows[0]);

    if ( i != exp_i)
    {
        Test->add_result(1, "Number of result sets is %llu instead of %llu\n", i, exp_i);
        return 1;
    }

    for (my_ulonglong j = 0; j < i; j++)
    {
        if (rows[j] != exp_rows[j])
        {
            Test->add_result(1, "For result set %llu number of rows is %llu instead of %llu\n", j, rows[j], exp_rows[j]);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{

    my_ulonglong *exp_rows = new my_ulonglong[10];

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



    Test->check_maxscale_alive();
    Test->copy_all_logs();

    return Test->global_result;

}
