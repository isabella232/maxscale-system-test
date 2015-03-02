/**
 * @file bug656.cpp Checks Maxscale behaviour in case if Master node is blocked
 *
 * - Connecto RWSplit
 * - block Mariadb server on Master node by Firewall
 * - try simple query *show servers" via Maxadmin
 */

#include <my_config.h>
#include "testconnections.h"
#include "maxadmin_operations.h"

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    int global_result = 0;

    Test->ReadEnv();
    Test->PrintIP();

    printf("Connecting to RWSplit %s\n", Test->Maxscale_IP);
    Test->ConnectRWSplit();

    printf("Setup firewall to block mysql on master\n"); fflush(stdout);
    Test->repl->BlockNode(0);

    //printf("Trying query to RWSplit, expecting failure, but not a crash\n"); fflush(stdout);
    //execute_query(Test->conn_rwsplit, (char *) "show processlist;");
    executeMaxadminCommandPrint(Test->Maxscale_IP, (char *) "admin", (char *) "skysql", (char *) "show servers");

    printf("Setup firewall back to allow mysql\n"); fflush(stdout);
    Test->repl->UnblockNode(0);

    sleep(10);

    global_result += CheckMaxscaleAlive();

    Test->CloseRWSplit();

    Test->Copy_all_logs(); return(global_result);
}

