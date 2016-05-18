/**
 * @file mxs118.cpp bug mxs118 regression case ("Two monitors loaded at the same time result into not working installation")
 *
 * - Configure two monitors using same backend serves
 * - try to connect to maxscale
 * - check logs for warning
 */


#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

int main(int argc, char *argv[])
{
    TestConnections * test = new TestConnections(argc, argv);
    test->stop_timeout();
    test->stop_maxscale();

    int rc1 = test->ssh_maxscale(true, "maxscale -c --user=maxscale ");
    test->tprintf("First test returned: %d\n", rc1);
    test->ssh_maxscale(true, "sed -e 's/service/ecivres/' /etc/maxscale.cnf");
    int rc2 = test->ssh_maxscale(true, "maxscale -c --user=maxscale ");
    test->tprintf("Second test returned: %d\n", rc2);
    test->add_result(rc1 == rc2, "Results should not be the same\n");
    test->copy_all_logs();
    return test->global_result;
}

