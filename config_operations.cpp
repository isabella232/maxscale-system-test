#include "config_operations.h"

Config::Config(TestConnections* parent):
    test(parent)
{
}

Config::~Config()
{
}

void Config::add_server(int num)
{
    test->tprintf("Adding the servers");

    for (int i = 0; i < 4; i++)
    {
        test->set_timeout(120);
        test->ssh_maxscale(true, "maxadmin add server server%d " MONITOR_NAME, num);
        test->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME1, num);
        test->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME2, num);
        test->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME3, num);
        test->stop_timeout();
    }
}

void Config::remove_server(int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin remove server server%d " MONITOR_NAME, num);
    test->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME1, num);
    test->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME2, num);
    test->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME3, num);
    test->stop_timeout();
}

void Config::destroy_server(int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin destroy server server%d", num);
    test->stop_timeout();
}

void Config::create_server(int num)
{
    test->set_timeout(120);
    test->ssh_maxscale(true, "maxadmin create server server%d %s",
                       num, test->repl->IP[num]);
    test->stop_timeout();
}
