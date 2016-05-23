
/**
 * @file mxs729_maxadmin.cpp Test of 'maxadmin' user add/delete
 *
 */


#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

using namespace std;

const char * only_root = "No administration users have been defined.\n";
const char * user_added = "User %s has been successfully added.\n";
const char * user_removed = "User %s has been successfully removed.\n";
const char * root_added = "User root has been successfully added.\n";
const char * user_and_root = "User names: %s, root\n";
const char * user_only = "User names: vagrant\n";

void add_remove_maxadmin_user(TestConnections* Test)
{
    char str[1024];

    Test->tprintf("add user %s to maxadmin:\n", Test->maxscale_access_user);
    char * st3 = Test->ssh_maxscale_output(TRUE, "maxadmin add user %s", Test->maxscale_access_user);
    Test->tprintf("Result: %s\n", st3);
    sprintf(str, user_added, Test->maxscale_access_user);
    if (strstr(st3, str) == NULL)
    {
        Test->add_result(1, "There is no proper '%s' message\n", str);
    } else {
        Test->tprintf("OK\n");
    }

    Test->tprintf("trying maxadmin without 'root':\n");
    char * st4 = Test->ssh_maxscale_output(FALSE, "maxadmin show users");
    Test->tprintf("Result: %s\n", st4);
    sprintf(str, user_only, Test->maxscale_access_user);
    if (strstr(st4, str) == NULL)
    {
        Test->add_result(1, "There is no proper '%s' message\n", str);
    } else {
        Test->tprintf("OK\n");
    }

    Test->tprintf("trying maxadmin with 'root':\n");
    int st5 = Test->ssh_maxscale(TRUE, "maxadmin show users");
    if (st5 == 0)
    {
        Test->add_result(1, "User added, but access to MaxAdmin as 'root' is still possible\n");
    } else {
        Test->tprintf("OK\n");
    }

    Test->tprintf("add user 'root':\n");
    char * st6 = Test->ssh_maxscale_output(FALSE, "maxadmin add user root");
    Test->tprintf("Result: %s\n", st6);
    if (strstr(st6, root_added) == NULL)
    {
        Test->add_result(1, "There is no proper '%s' message\n", str);
    } else {
        Test->tprintf("OK\n");
    }

    Test->tprintf("trying maxadmin without 'root'\n");
    char * st7 = Test->ssh_maxscale_output(FALSE, "maxadmin show users");
    Test->tprintf("Result: %s\n", st7);
    sprintf(str, user_and_root, Test->maxscale_access_user);
    if (strstr(st7, str) == NULL)
    {
        Test->add_result(1, "There is no proper '%s' message\n", str);
    } else {
        Test->tprintf("OK\n");
    }

    Test->tprintf("removing user '%s'\n", Test->maxscale_access_user);
    char * st8 = Test->ssh_maxscale_output(TRUE, "maxadmin remove user %s", Test->maxscale_access_user);
    Test->tprintf("trying maxadmin with 'root': %s\n", st8);
    sprintf(str, user_removed, Test->maxscale_access_user);
    if (strstr(st8, str) == NULL)
    {
        Test->add_result(1, "Wrong list of MaxAdmin users\n");
    } else {
        Test->tprintf("OK\n");
    }

    Test->tprintf("Trying with removed user '%s'\n", Test->maxscale_access_user);
    int st9 = Test->ssh_maxscale(FALSE, "maxadmin show users");
    if (st9 == 0)
    {
        Test->add_result(1, "User '%s'' removed, but access to MaxAdmin as '%s' is still possible\n", Test->maxscale_access_user, Test->maxscale_access_user);
    } else {
        Test->tprintf("OK\n");
    }
}

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(600);

    Test->ssh_maxscale(TRUE, "rm -rf /var/lib/maxscale/passwd");
    Test->restart_maxscale();

    Test->tprintf("trying maxadmin without 'root'\n");
    int st1 = Test->ssh_maxscale(FALSE, "maxadmin show users");
    Test->tprintf("exit code is: %d\n", st1);
    if (st1 == 0)
    {
        Test->add_result(1, "Access to MaxAdmin is possible without 'root' priveleges\n");
    }

    Test->tprintf("trying maxadmin with 'root'\n");
    char * st2 = Test->ssh_maxscale_output(TRUE, "maxadmin show users");
    Test->tprintf("Result: %s\n", st2);
    if (strstr(st2, only_root) == NULL)
    {
        Test->add_result(1, "Wrong list of MaxAdmin users\n");
    }

    add_remove_maxadmin_user(Test);

    Test->tprintf("trying long wierd user\n");
    char * st10 = Test->ssh_maxscale_output(TRUE, "maxadmin add user yygrgtrпрекури6н33имн756ККККЕН:УИГГГГ*?:*;:;*fj34oru34h275g23457g2v90590+u764gv56837fbv62381§SDFERGtrg45ergfergergefewfergt456ty");
    Test->tprintf("Result: %s\n", st10);
    if (strstr(st10, "has been successfully added") == NULL)
    {
        Test->add_result(1, "Wrong list of MaxAdmin users\n");
    }

    Test->check_maxscale_alive();
    Test->ssh_maxscale(TRUE, "rm -rf /var/lib/maxscale/passwd");

    Test->copy_all_logs(); return(Test->global_result);
}
