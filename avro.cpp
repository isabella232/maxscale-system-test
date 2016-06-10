/**
 * @file setup_binlog.cpp test of simple binlog router setup
 */

#include <my_config.h>
#include <iostream>
#include "testconnections.h"
#include "maxadmin_operations.h"
#include "sql_t1.h"

#include "test_binlog_fnc.h"


int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);
    Test->set_timeout(3000);
    Test->stop_maxscale();
    Test->ssh_maxscale(TRUE, (char *) "rm -rf /var/lib/maxscale/avro");

    //Test->ssh_maxscale(TRUE, (char *) "mkdir /var/lib/maxscale/avro; chown -R maxscale:maxscale /var/lib/maxscale/avro");

    Test->repl->connect();
    execute_query(Test->repl->nodes[0], (char *) "DROP TABLE IF EXISTS t1;");
    Test->repl->close_connections();
    sleep(5);


    Test->binlog_cmd_option = 0;
    Test->start_binlog();
    //test_binlog(Test);

    Test->repl->stop_node(1);
    Test->repl->stop_node(2);
    Test->repl->stop_node(3);

    Test->repl->connect();
    create_t1(Test->repl->nodes[0]);
    insert_into_t1(Test->repl->nodes[0], 3);
    Test->repl->close_connections();

    Test->copy_all_logs(); return(Test->global_result);
}

