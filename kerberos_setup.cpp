/**
 * @file kerberos_setup.cpp Attempt to configure KDC and node_000
 */


#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(1000);
    char str[1024];

    int i, j;
    for (i = 0; i < Test->repl->N; i++)
    {
        for (j = 0; j < Test->repl->N; j++)
        {
            sprintf(str, "sed -i \"$ a %s node_%03d.maxscale.test\" /etc/hosts", Test->repl->IP[j], j);
            Test->repl->ssh_node(i, str, true);

        }
        Test->repl->ssh_node(i, (char *) "yum install -y MariaDB-gssapi-server MariaDB-gssapi-client krb5-workstation pam_krb5", true);
        sprintf(str, "%s/krb5.conf", Test->test_dir);
        Test->repl->copy_to_node(str, (char *) "~/", i);
        Test->repl->ssh_node(i, (char *) "cp ~/krb5.conf /etc/", true);

    }

    Test->repl->ssh_node(0, (char *) "yum install rng-tools -y", true);
    Test->repl->ssh_node(0, (char *) "rngd -r /dev/urandom -o /dev/random", true);

    Test->repl->ssh_node(0, (char *) "yum install -y MariaDB-gssapi-server MariaDB-gssapi-client krb5-server krb5-workstation pam_krb5", true);

    Test->repl->ssh_node(0, (char *) "sed -i \"s/EXAMPLE.COM/MAXSCALE.TEST/\" /var/kerberos/krb5kdc/kdc.conf", true);
    Test->repl->ssh_node(0, (char *) "sed -i \"s/EXAMPLE.COM/MAXSCALE.TEST/\" /var/kerberos/krb5kdc/kadm5.acl", true);

    Test->repl->ssh_node(0, (char *) "kdb5_util create -P skysql -r MAXSCALE.TEST -s", true);
    Test->repl->ssh_node(0, (char *) "kadmin.local -q \"addprinc -pw skysql admin/admin@MAXSCALE.TEST\"", true);

    Test->repl->ssh_node(0, (char *) "iptables -I INPUT -p tcp --dport 749 -j ACCEPT", true);
    Test->repl->ssh_node(0, (char *) "iptables -I INPUT -p tcp --dport 88 -j ACCEPT", true);

    Test->repl->ssh_node(0, (char *) "service krb5kdc start", true);
    Test->repl->ssh_node(0, (char *) "service kadmin start", true);

    Test->repl->ssh_node(0, (char *) "echo \"skysql\" | sudo kadmin -p admin/admin -q \"addprinc -randkey mariadb/maxscale.test\"", true);


    for (i = 0; i < Test->repl->N; i++)
    {
//        Test->repl->ssh_node(i, (char *) "echo \"skysql\" | sudo kadmin -p admin/admin -q \"addprinc -randkey mariadb/maxscale.test\"", true);
        Test->repl->ssh_node(i, (char *) "echo \"skysql\" | sudo kadmin -p admin/admin -q \"ktadd mariadb/maxscale.test\"", true);

        Test->repl->ssh_node(i, (char *) "chmod a+r /etc/krb5.keytab;", true);

        sprintf(str, "%s/kerb.cnf", Test->test_dir);
        Test->repl->copy_to_node(str, (char *) "~/", i);
        Test->repl->ssh_node(i, (char *) "cp ~/kerb.cnf /etc/my.cnf.d/", true);
    }

    Test->repl->connect();
    Test->repl->execute_query_all_nodes((char *) "INSTALL SONAME 'auth_gssapi'");
    Test->repl->close_connections();


    Test->copy_all_logs(); return(Test->global_result);
}

