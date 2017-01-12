/**
 * @file
 */


#include <iostream>
#include <stdio.h>
#include "testconnections.h"
#include "test_binlog_fnc.h"

int get_first_binlog_file(TestConnections * Test, char * name, long long unsigned *size, char ** checksum)
{
    char size_str[64];
    char cmd[256];
    int res = 0;
    res  = find_field(Test->repl->nodes[0], (char *) "SHOW BINARY LOGS", (char *) "Log_name", name);
    res += find_field(Test->repl->nodes[0], (char *) "SHOW BINARY LOGS", (char *) "File_size", size_str);

    sscanf(size_str, "%llu", size);
    sprintf(cmd, "sha1sum /var/lib/mysql/%s | cut -f 1 -d \" \"", name);

    *checksum = Test->repl->ssh_node_output(0, cmd, true);

    Test->tprintf("First master binlog file:\nname: '%s'\nsize: %llu\nchecksum: %s\n",
                  name,
                  *size,
                  *checksum
                  );

    return res;
}

int main(int argc, char *argv[])
{
    setenv("no_maxscale_start", "yes", 1);
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(1000);
    char str1[1024];
    char str2[1024];

    int i;

    Test->repl->connect();
    Test->try_query(Test->repl->nodes[0], (char *) "DROP TABLE IF EXISTS t1");


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

    Test->repl->connect();

    Test->tprintf("Put some data to DB\n");
    Test->set_timeout(100);
    create_t1(Test->repl->nodes[0]);
    Test->add_result(insert_into_t1(Test->repl->nodes[0], 4), "Data inserting to t1 failed\n");
    Test->stop_timeout();
    Test->tprintf("Sleeping to let replication happen\n");
    sleep(60);

    for (i = 0; i < Test->repl->N; i++) {
        Test->tprintf("Checking data from node %d (%s)\n", i, Test->repl->IP[i]);
        Test->set_timeout(100);
        Test->add_result(select_from_t1(Test->repl->nodes[i], 4), "Selecting from t1 failed\n");
        Test->stop_timeout();
    }

    Test->tprintf("Flush logs\n");
    execute_query(Test->repl->nodes[0], (char *) "FLUSH LOGS");

    Test->tprintf("Running 'maxbinlogcheck' against Maxscale binlog file\n");
    char * maxscale_binlogcheck_output = Test->ssh_maxscale_output(true, "maxbinlogcheck -M -K /etc/mariadb_binlog_keys.txt -H /var/lib/maxscale/Binlog_Service/mar-bin.000001 2> 1");
    //puts(maxscale_binlogcheck_output);
    if (strstr(maxscale_binlogcheck_output, "error") != NULL)
    {
        Test->add_result(1, "Errors in the maxbinlogcheck output:\n%s\n", maxscale_binlogcheck_output);
    }

    if (strstr(maxscale_binlogcheck_output, "Decrypting binlog file with algorithm: aes_cbc") == NULL)
    {
        Test->add_result(1, "No 'Decrypting binlog file with algorithm: aes_cbc' in the maxbinlogcheck output:\n%s\n", maxscale_binlogcheck_output);
    }

    sprintf(str1, "mysqlbinlog -R -h %s -P %d -u%s -p%s mar-bin.000001 --stop-position=60000", Test->maxscale_IP, Test->binlog_port, Test->maxscale_user, Test->maxscale_password);
    Test->tprintf("running mysqlbinlog on node_000 to connecto Maxscale: %s\n", str1);
    //char * mysql_binlog_connect_output = Test->repl->ssh_node_output(0, str1, false);
    Test->add_result(Test->repl->ssh_node(0, str1, false), "Remote access to Maxscale binlog failed");
    //puts(mysql_binlog_connect_output);


    Test->tprintf("Checking binlog files on master\n");
    long long unsigned size_before;
    long long unsigned size_after;
    long long unsigned size_after_restart;
    char * checksum_before;
    char * checksum_after;
    char * checksum_after_restart;
    char name_before[256];
    char name_after[256];
    char name_after_restart[256];

    Test->add_result(get_first_binlog_file(Test, name_before, &size_before, &checksum_before),
                     "Error getting binlog name and size\n");

    Test->tprintf("Copying binlogs from Maxscale to Master\n");
    system("rm -rf binlogs");
    system("mkdir binlogs");
    Test->copy_from_maxscale((char *) "/var/lib/maxscale/Binlog_Service/*", (char *) "binlogs/");
    Test->repl->ssh_node(0, "rm -rf binlogs", true);
    Test->repl->copy_to_node("-r binlogs", "./", 0);
    Test->repl->ssh_node(0, "chown mysql:mysql binlogs/*", true);


    //Test->repl->ssh_node(0, "rm /var/lib/mysql/mar-bin*", true);
    Test->repl->ssh_node(0, "cp binlogs/* /var/lib/mysql/", true);
    sleep(5);
    Test->tprintf("Checking binlog files on master after copying binlogs from Maxscale\n");
    Test->add_result(get_first_binlog_file(Test, name_after, &size_after, &checksum_after),
                     "Error getting binlog name and size\n");

    Test->repl->close_connections();

    if (size_before != size_after)
    {
        Test->add_result(1, "Master binlog file size after copying Maxscale binlogs to Master is different\n");
    }
    if (strcmp(name_before, name_after) != 0)
    {
        Test->add_result(1, "Master binlog file name after copying Maxscale binlogs to Master is different\n");
    }
    if (strcmp(checksum_before, checksum_after) == 0)
    {
        Test->add_result(1, "Master binlog file checksum after copying Maxscale binlogs to Master is the same. Probably binlog copying error different\n");
    }

    Test->repl->stop_node(0);
    Test->repl->start_node(0, (char *) "");
    sleep(5);

    Test->tprintf("Checking binlog files on master after copying binlogs from Maxscale and Master restart\n");
    Test->repl->connect();
    Test->add_result(get_first_binlog_file(Test, name_after_restart, &size_after_restart, &checksum_after_restart),
                     "Error getting binlog name and size\n");
    Test->repl->close_connections();

    if (size_before != size_after_restart)
    {
        Test->add_result(1, "Master binlog file size after copying Maxscale binlogs to Master and restart is different\n");
    }
    if (strcmp(name_before, name_after_restart) != 0)
    {
        Test->add_result(1, "Master binlog file name after copying Maxscale binlogs to Master and restart is different\n");
    }
    if (strcmp(checksum_after_restart, checksum_after) != 0)
    {
        Test->add_result(1, "Master binlog file checksum after Master restart is different\n");
    }


    Test->copy_all_logs(); return(Test->global_result);
}
