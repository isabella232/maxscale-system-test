/**
 * @file fwf_reload - Same as fwf but with reloading of rules
 */


#include <iostream>
#include <ctime>
#include "testconnections.h"
#include "maxadmin_operations.h"
#include "sql_t1.h"
#include "fw_copy_rules.h"

int main(int argc, char *argv[])
{
    TestConnections *Test = new TestConnections(argc, argv);
    char sql[4096];
    char pass_file[4096];
    char deny_file[4096];
    char rules_dir[4096];

    sprintf(rules_dir, "%s/fw/", Test->test_dir);
    int N = 10;
    int i;

    Test->stop_maxscale();
    char first_rule[] = "rules1";
    copy_rules(Test, first_rule, rules_dir);
    Test->start_maxscale();
    Test->connect_rwsplit();


    for (i = 1; i <= N; i++)
    {
        char str[1024];
        sprintf(str, "rules%d", i);
        Test->set_timeout(180);
        copy_rules(Test, str, rules_dir);
        Test->ssh_maxscale(true, "maxadmin call function dbfwfilter rules/reload \"Database Firewall\"");

        int local_result = 0;
        sprintf(pass_file, "%s/fw/pass%d", Test->test_dir, i);
        FILE *file = fopen(pass_file, "r");

        if (file)
        {
            Test->tprintf("********** Trying queries that should be OK ********** \n");

            while (!feof(file))
            {
                Test->set_timeout(180);

                if (execute_query_from_file(Test->conn_rwsplit, file))
                {
                    Test->tprintf("Query should succeed: %s\n", sql);
                    local_result++;
                }
            }
            fclose(file);
        }
        else
        {
            Test->add_result(1, "Error opening file '%s': %d, %s\n", pass_file, errno, strerror(errno));
            break;
        }

        sprintf(deny_file, "%s/fw/deny%d", Test->test_dir, i);
        file = fopen(deny_file, "r");

        if (file)
        {
            Test->tprintf("********** Trying queries that should FAIL ********** \n");

            while (!feof(file))
            {
                Test->set_timeout(180);

                if (execute_query_from_file(Test->conn_rwsplit, file) == 0 ||
                    mysql_errno(Test->conn_rwsplit) != 1141)
                {
                    Test->tprintf("Query should fail: %s\n", sql);
                    local_result++;
                }
            }

            fclose(file);
        }
        else
        {
            Test->add_result(1, "Error opening file '%s': %d, %s\n", deny_file, errno, strerror(errno));
            break;
        }

        Test->add_result(local_result, "********** rules%d test FAILED\n", i);
    }

    Test->tprintf("Trying rules with syntax error\n");
    copy_rules(Test, (char *) "rules_syntax_error", rules_dir);

    int rc = Test->ssh_maxscale(true, "maxadmin call function dbfwfilter rules/reload \"Database Firewall\"");
    Test->add_result(rc == 0, "Reloading rules should fail with syntax errors");

    Test->check_maxscale_processes(1);
    Test->copy_all_logs();
    return Test->global_result;
}


