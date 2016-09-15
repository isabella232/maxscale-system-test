/**
 * @file bug448.cpp bug448 regression case ("Wildcard in host column of mysql.user table don't work properly")
 *
 * Test creates user1@xxx.%.%.% and tries to use it to connect
 */

/*
schulz 2014-06-19 10:06:07 UTC
Version 0.7.0 (wasn't in version list!)

When connecting via remote mysql client to maxscale host, there are problems with wildcard combinations in host definition of table mysql.user.

Using only "%" as host definition works.

Using a combination like "%.foo.bar" results in an error message like this:
2014 06/19 11:52:30   Error : getaddrinfo failed for [%.foo.bar] due [Name or service not known]
2014 06/19 11:52:30   140134673188672 [getUsers()] setipaddress failed: user test@%.foo.bar not added
Comment 1 Art van Scheppingen 2014-09-15 12:37:53 UTC
We have the same issue, but then with slightly differen functionality:
We white listed entire tenants in our Openstack environment using a schema as this:
192.168.8.% or 192.168.8.0/255.255.255.0
In both cases we get the same error as reported by Schulz.

Is this something you can fix by looping over the found ip addresses?

BTW: We can't use subdomains like Schulz as this would be too permissive in our setup.

Comment 2 Massimiliano 2014-09-23 13:11:38 UTC
Jira task is MAX-268

Comment 3 Massimiliano 2014-10-03 15:46:12 UTC
Added support for wildcard in ipv4 hosts:

current implementation works only with this syntax:


192.100.200.%
186.234.%.%
120.%.%.%


Example of users are printed via

show dbusers Test\ Service


User names: foo@%, repluser@%, joy@107.%.%.%, joy@107.0.0.%, going@107.170.19.%, joy@10.128.%.%, joy@10.128.0.%, four@10.128.214.%, hey@127.0.0.1, massi@127.0.0.1, repluser@127.0.0.1, going@192.168.1.%, hey@%, massi@%
*/


#include <my_config.h>
#include <iostream>
#include "testconnections.h"
#include "get_my_ip.h"

int main(int argc, char *argv[])
{
    char my_ip[1024];
    char my_ip_db[1024];
    char sql[1024];
    char * first_dot;
    TestConnections * Test = new TestConnections(argc, argv);

    Test->set_timeout(20);
    Test->repl->connect();

    get_my_ip(Test->maxscale_IP, my_ip);
    Test->tprintf("Test machine IP (got via network request) %s\n", my_ip);

    Test->add_result(Test->get_client_ip(my_ip_db), "Unable to get IP using connection to DB\n");

    Test->tprintf("Test machine IP (got via Show processlist) %s\n", my_ip);

    first_dot = strstr(my_ip, ".");
    strcpy(first_dot, ".%%.%%.%%");

    Test->tprintf("Test machine IP with %% %s\n", my_ip);

    Test->tprintf("Connecting to Maxscale\n");
    Test->add_result(Test->connect_maxscale(), "Error connecting to Maxscale\n");
    Test->tprintf("Creating user 'user1' for %s host\n", my_ip);
    Test->set_timeout(30);
    sprintf(sql, "CREATE USER user1@'%s';", my_ip);
    Test->tprintf("Query: %s\n", sql);
    Test->try_query(Test->conn_rwsplit, sql);

    sprintf(sql, "GRANT ALL PRIVILEGES ON *.* TO user1@'%s' identified by 'pass1';  FLUSH PRIVILEGES;", my_ip);
    Test->tprintf("Query: %s\n", sql);
    Test->try_query(Test->conn_rwsplit, sql);

    Test->tprintf("Trying to open connection using user1\n");

    MYSQL * conn = open_conn(Test->rwsplit_port, Test->maxscale_IP, (char *) "user1", (char *) "pass1", Test->ssl);
    if (mysql_errno(conn) != 0) {
        Test->add_result(1, "TEST_FAILED! Authentification failed! error: %s\n", mysql_error(conn));
    } else {
        Test->tprintf("Authentification for user@'%s' is ok", my_ip);
        if (conn != NULL) {mysql_close(conn);}
    }

    Test->add_result(execute_query(Test->conn_rwsplit, "DROP USER user1@'%s';  FLUSH PRIVILEGES;", my_ip), "Query Failed\n");

    Test->close_maxscale_connections();
    Test->check_maxscale_alive();

    Test->copy_all_logs(); return(Test->global_result);
}
