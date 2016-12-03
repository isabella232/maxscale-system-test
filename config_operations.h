#pragma once

#include "testconnections.h"

// The configuration should use these names for the services, listeners and monitors
#define MONITOR_NAME "mysql-monitor"
#define SERVICE_NAME1 "rwsplit-service"
#define SERVICE_NAME2 "read-connection-router-slave"
#define SERVICE_NAME3 "read-connection-router-master"
#define LISTENER_NAME1 "rwsplit-service-listener"
#define LISTENER_NAME2 "read-connection-router-slave-listener"
#define LISTENER_NAME3 "read-connection-router-master-listener"

class Config
{
    public:
    Config(TestConnections* parent);
    ~Config();

    /**
     * Add a server to all services and monitors
     * @param num Backend number
     */
    void add_server(int num);

    /**
     * Remove a server
     * @param num Backend number
     */
    void remove_server(int num);

    /**
     * Create a new server
     * @param num Backend number
     */
    void create_server(int num);

    /**
     * Destroy a server
     * @param num Backend number
     */
    void destroy_server(int num);

    private:
    TestConnections *test;
};
