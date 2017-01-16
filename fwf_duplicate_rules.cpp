/**
 * Dbfwfilter duplicate rule test
 *
 * Check if duplicate rules are detected.
 */

#include "testconnections.h"

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    test.ssh_maxscale_sh(true, "echo rule test1 deny no_where_clause > ~/rules/rules.txt;"
                         "echo rule test1 deny columns a b c >> ~/rules/rules.txt;"
                         "users %@% match any rules test1 >> ~/rules/rules.txt;");

    int rc = 0;

    if (test.restart_maxscale() == 0)
    {
        test.tprintf("Restarting MaxScale succeeded when it should've failed!");
        rc = 1;
    }

    return rc;
}
