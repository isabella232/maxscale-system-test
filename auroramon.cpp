/**
 * Test auroramon monitor
 */


#include "testconnections.h"
#include "rds_vpc_func.h"

int set_endspoints()
{
    char cmd[1024];
    char * result;

    json_t *root;
    json_error_t error;

    json_t * node;
    json_t *endpoint;
    long long int port;
    const char * IP;
    char p[64];



    for (int i = 0; i < 4; i++)
    {
        sprintf(cmd, "aws rds describe-db-instances --db-instance-identifier=node%03d", i);
        execute_cmd(cmd, &result);
        root = json_loads( result, 0, &error );
        if ( !root )
        {
            fprintf( stderr, "error: on line %d: %s\n", error.line, error.text );
            return -1;
        }
        node = json_array_get(json_object_get(root, "DBInstances"), 0);
        endpoint = json_object_get(node, "Endpoint");
        port = json_integer_value(json_object_get(endpoint, "Port"));
        IP = json_string_value(json_object_get(endpoint, "Address"));
        printf("host: %s \t port: %lld\n", IP, port);
        sprintf(cmd, "node_%03d_network", i);
        setenv(cmd, IP, 1);
        sprintf(cmd, "node_%03d_port", i);
        sprintf(p, "%lld", port);
        setenv(cmd, p, 1);
    }

    setenv("node_password", "skysqlrds", 1);
    setenv("maxscale_user", "skysql", 1);
    setenv("maxscale_password", "skysqlrds", 1);
    //system("env");

}

int do_failover(TestConnections* Test)
{
    char * result;
    const char * writer;
    const char * new_writer;
    get_writer(&writer);
    Test->tprintf("writer: %s\nDoing failover\n", writer);
    execute_cmd((char *) "aws rds failover-db-cluster --db-cluster-identifier=auroratest", &result);
    do
    {
        get_writer(&new_writer);
        Test->tprintf("writer: %s\n", new_writer);
        sleep(5);
    } while (strcmp(writer, new_writer) == 0);
    Test->tprintf("failover done\n");
    return(0);
}


void compare_masters(TestConnections* Test)
{
    const char * aurora_master;
    get_writer(&aurora_master);
    Test->tprintf("Aurora writer node: %s\n", aurora_master);
    char maxadmin_status[1024];
    int i;
    char cmd[1024];
    for (i = 0; i < Test->repl->N; i++)
    {
        sprintf(cmd, "show server server%d", i + 1);
        Test->get_maxadmin_param(cmd, (char *) "Status:", &maxadmin_status[0]);
        Test->tprintf("Server%d status %s\n", i + 1, maxadmin_status);
        sprintf(cmd, "node%03d", i);
        if (strcmp(aurora_master, cmd) == 0)
        {
            if (strcmp(maxadmin_status, "Master, Running"))
            {
                Test->tprintf("Maxadmin reports node%03d is a Master as expected", i);
            } else {
                Test->add_result(1, "Server node%03d status is not 'Master, Running'', it is '%s'", i, maxadmin_status);
            }
        } else {
            if (strcmp(maxadmin_status, "Slave, Running"))
            {
                Test->tprintf("Maxadmin reports node%03d is a Slave as expected", i);
            } else {
                Test->add_result(1, "Server node%03d status is not 'Slave, Running'', it is '%s'", i, maxadmin_status);
            }
        }

    }
}

int main(int argc, char *argv[])
{
    set_endspoints();

    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(30);

    compare_masters(Test);


    Test->set_timeout(30);
    Test->tprintf("Executing a query through readwritesplit before failover");
    Test->connect_rwsplit();
    Test->try_query(Test->conn_rwsplit, "show processlist");
    char server_id[1024];
    Test->tprintf("Get aurora_server_id\n");
    find_field(Test->conn_rwsplit, "select @@aurora_server_id;", "server_id", &server_id[0]);
    Test->close_rwsplit();
    Test->tprintf("server_id before failover: %s\n", server_id);

    Test->stop_timeout();
    Test->tprintf("Performing cluster failover");

    do_failover(Test);

    // Do the failover here and wait until it is over
    //sleep(10);

    Test->set_timeout(30);
    Test->tprintf("Executing a query through readwritesplit after failover");
    Test->connect_rwsplit();
    Test->try_query(Test->conn_rwsplit, "show processlist");
    Test->tprintf("Get aurora_server_id\n");
    find_field(Test->conn_rwsplit, "select @@aurora_server_id;", "server_id", &server_id[0]);
    Test->close_rwsplit();
    Test->tprintf("server_id after failover: %s\n", server_id);

    compare_masters(Test);

    //Test->check_maxscale_alive();
    //Test->copy_all_logs();
    return Test->global_result;

}

