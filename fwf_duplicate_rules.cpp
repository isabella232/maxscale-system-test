/**
 * Dbfwfilter duplicate rule test
 *
 * Check if duplicate rules are detected.
 */

#include "testconnections.h"

int main(int argc, char** argv)
{
    TestConnections test(argc, argv);
    test.ssh_maxscale_sh(true, "mkdir -p /home/vagrant/rules/"
                         "echo rule test1 deny no_where_clause > /home/vagrant/rules/rules.txt;"
                         "echo rule test1 deny columns a b c >> /home/vagrant/rules/rules.txt;"
                         "users %@% match any rules test1 >> /home/vagrant/rules/rules.txt;"
                         "chmod a+r /home/vagrant/rules/rules.txt;");

    int rc = 0;

    if (test.restart_maxscale() == 0)
    {
        test.tprintf("Restarting MaxScale succeeded when it should've failed!");
        rc = 1;
    }

    return rc;
}
