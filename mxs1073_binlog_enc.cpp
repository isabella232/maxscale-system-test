/**
 * @file
 */


#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    setenv("no_maxscale_start", "yes", 1);
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(1000);
    char str1[1024];
    char str2[1024];

    int i;


    Test->tprintf("Coping encription config .cnf files to all nodes \n");
    sprintf(str1, "%s/binlog_enc.cnf", Test->test_dir);
    sprintf(str2, "%s/mariadb_binlog_keys.txt", Test->test_dir);
    for (i = 0; i < Test->repl->N; i++)
    {
        Test->repl->copy_to_node(str1, (char *) "~/", i);
        Test->repl->ssh_node(i, (char *) "cp ~/binlog_enc.cnf /etc/my.cnf.d/", true);

        Test->repl->copy_to_node(str2, (char *) "~/", i);
        Test->repl->ssh_node(i, (char *) "cp ~/mariadb_binlog_keys.txt /etc/", true);
    }

    Test->copy_to_maxscale(str2, (char *) "~/");
    Test->ssh_maxscale(true, "cp ~/mariadb_binlog_keys.txt /etc/");

    Test->start_binlog();


    Test->copy_all_logs(); return(Test->global_result);
}
