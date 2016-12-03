#include "config_operations.h"

// The configuration should use these names for the services, listeners and monitors
#define MONITOR_NAME "mysql-monitor"
#define SERVICE_NAME1 "rwsplit-service"
#define SERVICE_NAME2 "read-connection-router-master"
#define SERVICE_NAME3 "read-connection-router-slave"
#define LISTENER_NAME1 "rwsplit-service-listener"
#define LISTENER_NAME2 "read-connection-router-master-listener"
#define LISTENER_NAME3 "read-connection-router-slave-listener"

struct
{
    const char *service;
    const char *listener;
    int         port;
} services[]
{
    {SERVICE_NAME1, LISTENER_NAME1, 4006},
    {SERVICE_NAME2, LISTENER_NAME2, 4008},
    {SERVICE_NAME3, LISTENER_NAME3, 4009}
};

Config::Config(TestConnections* parent):
    test_(parent)
{
}

Config::~Config()
{
}

void Config::add_server(int num)
{
    test_->tprintf("Adding the servers");
    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin add server server%d " MONITOR_NAME, num);
    test_->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME1, num);
    test_->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME2, num);
    test_->ssh_maxscale(true, "maxadmin add server server%d " SERVICE_NAME3, num);
    test_->stop_timeout();
}

void Config::remove_server(int num)
{
    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin remove server server%d " MONITOR_NAME, num);
    test_->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME1, num);
    test_->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME2, num);
    test_->ssh_maxscale(true, "maxadmin remove server server%d " SERVICE_NAME3, num);
    test_->stop_timeout();
}

void Config::destroy_server(int num)
{
    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin destroy server server%d", num);
    created_servers_.erase(num);
    test_->stop_timeout();
}

void Config::create_server(int num)
{
    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin create server server%d %s",
                       num, test_->repl->IP[num]);
    created_servers_.insert(num);
    test_->stop_timeout();
}

void Config::alter_server(int num, const char *key, const char *value)
{
    test_->ssh_maxscale(true, "maxadmin alter server server%d %s=%s", num, key, value);
}

void Config::alter_server(int num, const char *key, int value)
{
    test_->ssh_maxscale(true, "maxadmin alter server server%d %s=%d", num, key, value);
}

void Config::alter_server(int num, const char *key, float value)
{
    test_->ssh_maxscale(true, "maxadmin alter server server%d %s=%f", num, key, value);
}

void Config::create_monitor(const char *module, int interval)
{
    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin create monitor " MONITOR_NAME " %s", module);
    alter_monitor("monitor_interval", interval);
    test_->ssh_maxscale(true, "maxadmin restart monitor " MONITOR_NAME);
    test_->stop_timeout();
}

void Config::alter_monitor(const char *key, const char *value)
{
    test_->ssh_maxscale(true, "maxadmin alter monitor " MONITOR_NAME " %s=%s", key, value);
}

void Config::alter_monitor(const char *key, int value)
{
    test_->ssh_maxscale(true, "maxadmin alter monitor " MONITOR_NAME " %s=%d", key, value);
}

void Config::alter_monitor(const char *key, float value)
{
    test_->ssh_maxscale(true, "maxadmin alter monitor " MONITOR_NAME " %s=%f", key, value);
}

void Config::destroy_monitor()
{
    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin destroy monitor " MONITOR_NAME);
    test_->stop_timeout();
}

void Config::create_listener(Config::Service service)
{
    int i = static_cast<int>(service);

    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin create listener %s %s 0.0.0.0 %d",
                       services[i].service,
                       services[i].listener,
                       services[i].port);
    test_->stop_timeout();
}

void Config::destroy_listener(Config::Service service)
{
    int i = static_cast<int>(service);

    test_->set_timeout(120);
    test_->ssh_maxscale(true, "maxadmin destroy listener %s %s",
                       services[i].service, services[i].listener);
    test_->stop_timeout();
}

void Config::create_all_listeners()
{
    create_listener(SERVICE_RWSPLIT);
    create_listener(SERVICE_RCONN_SLAVE);
    create_listener(SERVICE_RCONN_MASTER);
}

void Config::reset()
{
    /** Make sure the servers exist before checking that connectivity is OK */
    for (int i = 0; i < test_->repl->N; i++)
    {
        if (created_servers_.find(i) == created_servers_.end())
        {
            create_server(i);
            add_server(i);
        }
    }
}
