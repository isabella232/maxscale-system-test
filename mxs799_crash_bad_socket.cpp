/**
 * @file mxs799_crash_bad_socket.cpp mxs710_bad_socket regression case ("Maxscale does not startup properly and crashes after trying to login to database")
 * - try to start maxscale with "socket=/var/run/mysqld/mysqld.sock" in the listener definition
 * - try to connect listener
 * - do not expect crash
 */


#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->connect_maxscale();
    Test->close_maxscale_connections();

    Test->check_maxscale_alive();
    Test->copy_all_logs(); return(Test->global_result);
}
