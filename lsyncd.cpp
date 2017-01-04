/**
 * @file lsyncd.cpp Test of lsyncd setup to synchronize maxscale.cnf
 */

#include <iostream>
#include <unistd.h>
#include "testconnections.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cofigure_lsyncd(TestConnections * Test)
{
    char cmd[1024];
    int r = 0;
    int i;
    sprintf(cmd, "cp %s/lsyncd.conf.template lsyncd.conf", Test->test_dir);
    system(cmd);
    for (i = 0; i < Test->repl->N; i++)
    {
        sprintf(cmd,
                "sed -i \"s/###node_IP_%d###/%s/\" lsyncd.conf",
                i + 1,
                Test->repl->IP[i]);
        r += system(cmd);
        sprintf(cmd,
                "sed -i \"s/###node_user_%d###/%s/\" lsyncd.conf",
                i + 1,
                Test->repl->access_user[i]);
        r += system(cmd);
    }
    for (i = 0; i < Test->repl->N; i++)
    {
        r += Test->repl->copy_to_node("lsyncd.conf", "./", i);
        r += Test->repl->ssh_node(i, "cp lsyncd.conf /etc/", true);
    }
    return(r);
}

int install_lsyncd(TestConnections * Test, int node_index)
{
    Test->tprintf("Installing lsyncd\n");
    Test->tprintf("Trying yum\n");
    Test->repl->ssh_node(node_index, (char *) "yum install -y wget", true);
    Test->repl->ssh_node(node_index, (char *) "wget http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-8.noarch.rpm", true);
    Test->repl->ssh_node(node_index, (char *) "rpm -ivh epel-release-7-8.noarch.rpm", true);

    Test->repl->ssh_node(node_index, (char *) "mkdir -p /var/log/maxscale", true);

    if (Test->repl->ssh_node(node_index, (char *) "yum install -y lsyncd", true) == 0)
    {
        Test->tprintf("Done\n");
        return 0;
    }
    Test->tprintf("Trying apt\n");
    Test->repl->ssh_node(node_index, (char *) "apt-get update", true);
    if (Test->repl->ssh_node(node_index, (char *) "apt-get install -y --force-yes lsyncd", true) == 0)
    {
        Test->tprintf("Done\n");
        return 0;
    }
    Test->tprintf("Trying zypper\n");
    if (Test->repl->ssh_node(node_index, (char *) "zypper -n install lsyncd", true)  == 0)
    {
        Test->tprintf("Done\n");
        return 0;
    }
    return 1;
}


int main(int argc, char *argv[])
{
    int i;
    setenv("no_maxscale_start", "yes", 1);
    static TestConnections * Test = new TestConnections(argc, argv);

    Test->tprintf("Generating ssh keys\n");
    Test->repl->ssh_node(0, "ssh-keygen -t rsa -N \"\" -f .ssh/id_rsa", false);

    Test->tprintf("Copying ssh keys\n");
    Test->repl->copy_from_node(".ssh/id_rsa*", ".", 0);
    for (i = 1; i < Test->repl->N; i++)
    {
        Test->repl->copy_to_node("id_rsa*", ".ssh/", i);
        Test->repl->ssh_node(i, "cat .ssh/id_rsa.pub >> .ssh/authorized_keys", false);

        // HACK
        Test->repl->ssh_node(i, "chmod 777 -R /etc", true);
    }


    Test->add_result(install_lsyncd(Test, 0), "Failed to install lsyncd on node 0\n");
    Test->add_result(install_lsyncd(Test, 1), "Failed to install lsyncd on node 1\n");

    Test->tprintf("Configuring lsyncd on all nodes\n");
    Test->add_result(cofigure_lsyncd(Test), "Failed to create and copy lsyncd.conf\n");

    Test->tprintf("Starting lsyncd on all nodes\n");
    for (i = 0; i < Test->repl->N; i++)
    {
        Test->repl->ssh_node(i, "service lsyncd start", true);
    }


    Test->copy_all_logs();
    return Test->global_result;
}


