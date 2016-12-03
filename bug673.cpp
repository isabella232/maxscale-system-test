/**
 * @file bug673.cpp  regression case for bug673 ("MaxScale crashes if "Users table data" is empty and "show dbusers" is executed in maxadmin")
 *
 * - configure wrong IP for all backends
 * - execute maxadmin command show dbusers "RW Split Router"
 * - check MaxScale is alive by executing maxadmin again
 */

/*

Description Kolbe Kegel 2014-12-30 23:09:22 UTC
Error : Unable to get user data from backend database for service [RW Split Router]. Missing server information.
Error : Unable to load users from 0.0.0.0:4006 for service RW Split Router.
Error : Failed to start service 'RW Split Router'.

MaxScale> show dbusers "RW Split Router"
Users table data
[root@max1 maxscale]#
Comment 1 Kolbe Kegel 2014-12-30 23:11:28 UTC
Dec 30 15:08:23 max1 MaxScale[19483]: Fatal: MaxScale received fatal signal 11. Attempting backtrace.
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale() [0x54912e]
Dec 30 15:08:23 max1 MaxScale[19483]:   /lib64/libpthread.so.0(+0xf710) [0x7fb6fa55b710]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale(spinlock_acquire+0x1d) [0x548e05]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale() [0x55e4ab]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale(hashtable_get_stats+0x40) [0x55e3fc]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale(dcb_hashtable_stats+0x3a) [0x550157]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale(dcb_usersPrint+0x3e) [0x55d8eb]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/modules/libcli.so(execute_cmd+0x996) [0x7fb6f42d82d1]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/modules/libcli.so(+0x557b) [0x7fb6f42d757b]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/modules/libmaxscaled.so(+0x319e) [0x7fb6e042419e]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale() [0x558168]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale(poll_waitevents+0x63d) [0x557a21]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale(main+0x18d1) [0x54baab]
Dec 30 15:08:23 max1 MaxScale[19483]:   /lib64/libc.so.6(__libc_start_main+0xfd) [0x7fb6f8dfad5d]
Dec 30 15:08:23 max1 MaxScale[19483]:   /usr/local/skysql/maxscale/bin/maxscale() [0x547ff1]
Comment 2 Vilho Raatikka 2014-12-31 14:52:03 UTC
(In reply to comment #0)
> Error : Unable to get user data from backend database for service [RW Split
> Router]. Missing server information.
> Error : Unable to load users from 0.0.0.0:4006 for service RW Split Router.

How did you get this error occur?
Comment 3 Vilho Raatikka 2014-12-31 15:08:17 UTC
No NULL check in hashtable.c:hashtable_get_stats and division by zero in dcb.c:dcb_hashtable_stats
Comment 4 Vilho Raatikka 2014-12-31 15:09:24 UTC
Reproduce by using invalid server specification in MaxScale.cnf, then connect with maxadmin and execute show dbusers "RW Split Router" << use correct name.
Comment 5 Vilho Raatikka 2014-12-31 19:08:15 UTC
dcb.c:dcb_hashtable_stats:division by zero
    hashtable.c:hashtable_get_stats: NULL-pointer reference
    service.c:serviceStartPort:set service->users NULL to avoid referring to freed memory
    users.c:dcb_usersPrintf: NULL-pointer reference
    debugcmd.c:convert_arg: changed return value to 1 in case of error, 0 (==NULL) is valid but it indicates that there are no users loaded.
        execute_cmd: fixed command handling

*/


#include "testconnections.h"
#include "maxadmin_operations.h"

int main(int argc, char *argv[])
{
    char result[1024];
    TestConnections * Test = new TestConnections(argc, argv);

    sleep(30);

    Test->set_timeout(20);

    Test->tprintf("Trying show dbusers \"RW Split Router\"\n");
    Test->add_result(Test->get_maxadmin_param((char *) "show dbusers \"RW Split Router\"", (char *) "User names:", result), "Maxadmin failed\n");
    Test->tprintf("result %s\n", result);

    Test->tprintf("Trying show dbusers \"Read Connection Router Master\"\n");
    Test->add_result(Test->get_maxadmin_param((char *) "show dbusers \"Read Connection Router Master\"", (char *) "User names:", result), "Maxadmin failed\n");
    Test->tprintf("result %s\n", result);


    Test->tprintf("Trying show dbusers \"Read Connection Router Slave\"\n");
    Test->add_result(Test->get_maxadmin_param((char *) "show dbusers \"Read Connection Router Slave\"", (char *) "User names:", result), "Maxadmin failed\n");
    Test->tprintf("result %s\n", result);


    Test->tprintf("Trying again show dbusers \"RW Split Router\" to check if MaxScale is alive\n");
    Test->add_result(Test->get_maxadmin_param((char *) "show dbusers \"RW Split Router\"", (char *) "User names:", result), "Maxadmin failed\n");
    Test->tprintf("result %s\n", result);

    Test->copy_all_logs(); return(Test->global_result);
}
