/**
 * @file kerberos_setup.cpp Attempt to configure KDC and node_000
 */


#include <iostream>
#include "testconnections.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(10);
    char str[1024];

    int i, j;
    for (i = 0; i < Test->repl->N; i++)
    {
        for (j = 0; j < Test->repl->N; j++)
        {
            sprintf(str, "echo %s node_%3d >> /etc/hosts\n", Test->repl->IP[j], j);
            Test->repl->ssh_node(i, str, true);
        }
    }

    Test->copy_all_logs(); return(Test->global_result);
}

