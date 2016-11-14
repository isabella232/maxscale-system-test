/**
 * @file mxs822_maxpasswd.cpp Regression test for bug MXS-822 ("Session freeze when small tail packet")
 * - create .secret with maxkeys
 * - generate encripted password with maxpasswd, use password with special characters
 * - replace passwords in maxscale.cnf with generated encripted password
 * - try to connect to RWSplit
 * - restore passwords in maxscale.cnf
 * - repeate for several other password with special characters
 */



#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

void try_password(TestConnections* Test, char * pass)
{
    char sql[256];
    sprintf(sql, "grant all privileges on *.* to skysql@'%%%%' identified by '%s';", pass);
    Test->tprintf("Changing password: %s\n", sql);

    Test->connect_maxscale();
    Test->try_query(Test->conn_rwsplit,  sql);

    sprintf(sql, "grant all privileges on *.* to maxskysql@'%%%%' identified by '%s';", pass);
    Test->tprintf("Changing password: %s\n", sql);
    Test->try_query(Test->conn_rwsplit,  sql);

    Test->close_maxscale_connections();

    Test->tprintf("Executing 'maxkeys'\n");
    Test->ssh_maxscale(TRUE, "maxkeys");

    Test->ssh_maxscale(TRUE, "sudo chown maxscale:maxscale /var/lib/maxscale/.secrets");

    Test->tprintf("Encrypting password '%s'\n", pass);

    Test->set_timeout(30);
    char * enc_pass = Test->ssh_maxscale_output(TRUE, "maxpasswd %s | tr -cd \"[:print:]\" ", pass);

    Test->tprintf("Encripted password: %s\n", enc_pass);

    Test->set_timeout(30);
    Test->ssh_maxscale(TRUE, "sed -i \"s/passwd=skysql/passwd=%s/\" /etc/maxscale.cnf", enc_pass);

    Test->tprintf("sed \"s/passwd=skysql/passwd=%s/\" ", enc_pass);

    Test->set_timeout(30);
    Test->restart_maxscale();

    MYSQL * conn = open_conn(Test->rwsplit_port, Test->maxscale_IP, Test->maxscale_user, pass, Test->ssl);

    sprintf(sql, "grant all privileges on *.* to skysql@'%%%%' identified by '%s';", Test->maxscale_password);
    Test->tprintf("Changing password: %s\n", sql);

    Test->try_query(conn,  sql);

    sprintf(sql, "grant all privileges on *.* to maxskysql@'%%%%' identified by '%s';", Test->maxscale_password);
    Test->tprintf("Changing password: %s\n", sql);
    Test->try_query(conn,  sql);

    mysql_close(conn);

    Test->tprintf("Restoring password: %s\n", sql);
    Test->ssh_maxscale(TRUE, "sed -i \"s/passwd=%s/passwd=skysql/\" /etc/maxscale.cnf", enc_pass);

    Test->restart_maxscale();
}

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(30);

    try_password(Test, (char *) "aaa$aaa");
    try_password(Test, (char *) "#¤&");
    try_password(Test, (char *) "пароль");

    /*
    try_password(Test, (char *) "p1");
    try_password(Test, (char *) "p2");
    try_password(Test, (char *) "p3");
    */

    Test->check_maxscale_alive();
    Test->copy_all_logs(); return(Test->global_result);
}
