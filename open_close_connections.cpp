/**
 * @file open_close_connections.cpp Simple test which creates load which is very short sessions
 *
 * - 20 threads are opening and immediatelly closing connection in the loop
 */

#include "testconnections.h"

typedef struct
{
    int exit_flag;
    int thread_id;
    long i;
    int rwsplit_only;
    TestConnections * Test;
} openclose_thread_data;

void *query_thread1(void *ptr);
int threads_num = 20;

int main(int argc, char *argv[])
{
    TestConnections * Test = new TestConnections(argc, argv);

    // Tuning these kernel parameters removes any system limitations on
    // how many connections can be created within a short period
    Test->ssh_maxscale(true, "sysctl net.ipv4.tcp_tw_reuse=1 net.ipv4.tcp_tw_recycle=1 "
                       "net.core.somaxconn=10000 net.ipv4.tcp_max_syn_backlog=10000");

    Test->set_timeout(20);

    openclose_thread_data data[threads_num];

    int run_time = 300;

    if (Test->smoke)
    {
        run_time = 10;
    }

    for (int i = 0; i < threads_num; i++)
    {
        data[i].i = 0;
        data[i].exit_flag = 0;
        data[i].Test = Test;
        data[i].rwsplit_only = 1;
        data[i].thread_id = i;
    }

    pthread_t thread1[threads_num];

    int  iret1[threads_num];

    Test->repl->execute_query_all_nodes((char *) "set global max_connections = 50000;");
    Test->repl->sync_slaves();

    /* Create independent threads each of them will execute function */
    for (int i = 0; i < threads_num; i++)
    {
        iret1[i] = pthread_create( &thread1[i], NULL, query_thread1, &data[i]);
    }

    Test->tprintf("Threads are running %d seconds \n", run_time);

    for (int i = 0; i < threads_num; i++)
    {
        data[i].rwsplit_only = 1;
    }

    Test->set_timeout(run_time + 20);
    sleep(run_time);

    Test->tprintf("all routers are involved, threads are running %d seconds more\n", run_time);
    Test->set_timeout(run_time + 40);

    for (int i = 0; i < threads_num; i++)
    {
        data[i].rwsplit_only = 0;
    }

    sleep(run_time);

    for (int i = 0; i < threads_num; i++)
    {
        data[i].exit_flag = 1;
        pthread_join(iret1[i], NULL);
    }

    Test->stop_timeout();
    Test->check_maxscale_alive();
    Test->copy_all_logs();
    return Test->global_result;
}

void *query_thread1( void *ptr )
{
    unsigned int m_errno;
    openclose_thread_data * data = (openclose_thread_data *) ptr;
    MYSQL * conn_node[data->Test->repl->N];
    int rw_o = data->rwsplit_only;

    for (int k = 0; k < data->Test->repl->N; k++)
    {
        conn_node[k] = open_conn(data->Test->repl->port[k],
                                 data->Test->repl->IP[k],
                                 data->Test->repl->user_name,
                                 data->Test->repl->password,
                                 data->Test->repl->ssl);
    }

    while (data->exit_flag == 0 && data->Test->global_result == 0)
    {
        MYSQL *conn1 = data->Test->open_rwsplit_connection();
        m_errno = mysql_errno(conn1);

        if (m_errno != 0)
        {
            data->Test->add_result(1, "Error opening RWsplit conn, thread num is %d, iteration %d, error is: %s\n",
                                   data->thread_id, data->i, mysql_error(conn1));
            for (int k = 0; k < data->Test->repl->N; k++)
            {
                int  n_conn = get_conn_num(conn_node[k], data->Test->maxscale_IP,
                                           data->Test->maxscale_hostname, (char *) "test");
                data->Test->tprintf("conn to node%d is %d (thread is %d)\n", k, n_conn, data->thread_id);
            }
        }

        if (rw_o == 0)
        {
            MYSQL *conn2 = data->Test->open_readconn_master_connection();
            data->Test->add_result(mysql_errno(conn2),
                                   "Error opening ReadConn master conn, thread num is %d, iteration %d, error is: %s\n", data->thread_id,
                                   data->i, mysql_error(conn2));
            MYSQL *conn3 = data->Test->open_readconn_slave_connection();
            data->Test->add_result(mysql_errno(conn3),
                                   "Error opening ReadConn master conn, thread num is %d, iteration %d, error is: %s\n", data->thread_id,
                                   data->i, mysql_error(conn3));
            if (conn2 != NULL)
            {
                data->Test->try_query(conn2, (char*) "USE test");
                mysql_close(conn2);
            }
            if (conn3 != NULL)
            {
                data->Test->try_query(conn3, (char*) "USE test");
                mysql_close(conn3);
            }

        }

        // USE test here is a hack to prevent Maxscale from failure; should be removed when fixed
        if (conn1 != NULL)
        {
            data->Test->try_query(conn1, (char*) "USE test");
            mysql_close(conn1);
        }

        data->i++;
    }

    for (int k = 0; k < data->Test->repl->N; k++)
    {
        mysql_close(conn_node[k]);
    }

    return NULL;
}
