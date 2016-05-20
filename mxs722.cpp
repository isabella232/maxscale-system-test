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

    /** Get a baseline result with a good configuration */
    int baseline = test->ssh_maxscale(true, "maxscale -c --user=maxscale ");

    /** Create a type error in the configuration and see if it is noticed */
    test->ssh_maxscale(true, "sed -i -e 's/service/ecivres/' /etc/maxscale.cnf");
    test->add_result(baseline == test->ssh_maxscale(true, "maxscale -c --user=maxscale "),
                     "Configuration error should be detected.\n");
    test->ssh_maxscale(true, "sed -i -e 's/ecivres/service/' /etc/maxscale.cnf");

    test->copy_all_logs();
    return test->global_result;
}
