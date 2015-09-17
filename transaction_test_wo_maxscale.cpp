/**
 * @file transaction_test_wo_maxscale.cpp
 */

#include <my_config.h>
#include <iostream>
#include "testconnections.h"
#include "maxadmin_operations.h"
#include "sql_t1.h"

int check_sha1(TestConnections* Test)
{
    char sys[1024];
    char * x;
    FILE *ls;
    int global_result = 0;
    int i;

    char buf[1024];
    char buf_max[1024];

    printf("ls before FLUSH LOGS\n");

    printf("Master");fflush(stdout);
    sprintf(sys, "ssh -i %s -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null %s@%s 'ls -la /var/lib/mysql/mar-bin.0000*'", Test->repl->sshkey[0], Test->repl->access_user[0], Test->repl->IP[0]);
    system(sys);

    printf("FLUSH LOGS\n");fflush(stdout);
    global_result += execute_query(Test->repl->nodes[0], (char *) "FLUSH LOGS");
    printf("Logs flushed\n");
    sleep(20);
    printf("ls after first FLUSH LOGS\n");

    printf("Master\n");fflush(stdout);
    sprintf(sys, "ssh -i %s -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null %s@%s 'ls -la /var/lib/mysql/mar-bin.00000*'", Test->repl->sshkey[0], Test->repl->access_user[0], Test->repl->IP[0]);
    system(sys);


    printf("FLUSH LOGS\n");fflush(stdout);
    global_result += execute_query(Test->repl->nodes[0], (char *) "FLUSH LOGS");
    printf("Logs flushed\n"); fflush(stdout);

    sleep(19);
    printf("ls before FLUSH LOGS\n");

    printf("Master");
    sprintf(sys, "ssh -i %s -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null %s@%s 'ls -la /var/lib/mysql/mar-bin.00000*'", Test->repl->sshkey[0], Test->repl->access_user[0], Test->repl->IP[0]);
    system(sys);fflush(stdout);


    return(global_result);
}

int start_transaction(TestConnections* Test)
{
    int global_result = 0;
    printf("Transaction test\n");
    printf("Start transaction\n");
    global_result += execute_query(Test->repl->nodes[0], (char *) "START TRANSACTION");
    //global_result += execute_query(Test->repl->nodes[0], (char *) "SET autocommit = 0");
    printf("INSERT data\n");
    global_result += execute_query(Test->repl->nodes[0], (char *) "INSERT INTO t1 VALUES(111, 10)");
    sleep(20);
    return(global_result);
}

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int global_result = 0;

    int i;

    Test->read_env();
    Test->print_env();

    for (int option = 0; option < 3; option++) {

        Test->repl->connect();

        create_t1(Test->repl->nodes[0]);
        global_result += insert_into_t1(Test->repl->nodes[0], 4);
        printf("Sleeping to let replication happen\n"); fflush(stdout);
        sleep(30);

        for (i = 0; i < Test->repl->N; i++) {
            printf("Checking data from node %d (%s)\n", i, Test->repl->IP[i]); fflush(stdout);
            global_result += select_from_t1(Test->repl->nodes[i], 4);
        }

        printf("First transaction test (with ROLLBACK)\n");
        start_transaction(Test);

        printf("SELECT * FROM t1 WHERE fl=10, checking inserted values\n");
        global_result += execute_query_check_one(Test->repl->nodes[0], (char *) "SELECT * FROM t1 WHERE fl=10", "111");

        //printf("SELECT, checking inserted values from slave\n");
        //global_result += execute_query_check_one(Test->repl->nodes[2], (char *) "SELECT * FROM t1 WHERE fl=10", "111");

        global_result += check_sha1(Test);

        printf("ROLLBACK\n");
        global_result += execute_query(Test->repl->nodes[0], (char *) "ROLLBACK");
        printf("INSERT INTO t1 VALUES(112, 10)\n");
        global_result += execute_query(Test->repl->nodes[0], (char *) "INSERT INTO t1 VALUES(112, 10)");
        sleep(20);

        printf("SELECT * FROM t1 WHERE fl=10, checking inserted values\n");
        global_result += execute_query_check_one(Test->repl->nodes[0], (char *) "SELECT * FROM t1 WHERE fl=10", "112");

        printf("SELECT * FROM t1 WHERE fl=10, checking inserted values from slave\n");
        global_result += execute_query_check_one(Test->repl->nodes[2], (char *) "SELECT * FROM t1 WHERE fl=10", "112");
        printf("DELETE FROM t1 WHERE fl=10\n");
        global_result += execute_query(Test->repl->nodes[0], (char *) "DELETE FROM t1 WHERE fl=10");
        printf("Checking t1\n");
        global_result += select_from_t1(Test->repl->nodes[0], 4);

        printf("Second transaction test (with COMMIT)\n");
        start_transaction(Test);

        printf("COMMIT\n");
        global_result += execute_query(Test->repl->nodes[0], (char *) "COMMIT");

        printf("SELECT, checking inserted values\n");
        global_result += execute_query_check_one(Test->repl->nodes[0], (char *) "SELECT * FROM t1 WHERE fl=10", "111");

        printf("SELECT, checking inserted values from slave\n");
        global_result += execute_query_check_one(Test->repl->nodes[2], (char *) "SELECT * FROM t1 WHERE fl=10", "111");
        printf("DELETE FROM t1 WHERE fl=10\n");
        global_result += execute_query(Test->repl->nodes[0], (char *) "DELETE FROM t1 WHERE fl=10");

        global_result += check_sha1(Test);
        Test->repl->close_connections();

    }


    Test->copy_all_logs(); return(global_result);
}
