
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <openssl/sha.h>
#include "maxinfo_func.h"
#include "sql_t1.h"
#include <sys/epoll.h>

using namespace std;
char reg_str[] = "REGISTER UUID=XXX-YYY_YYY, TYPE=JSON";
char req_str[] = "REQUEST-DATA test.t1";
int insert_val = 0;
bool exit_flag = false;

void *query_thread(void *ptr);

bool cdc_com(TestConnections *Test)
{
    int max_inserted_val = Test->smoke ? 25 : 100;
    int sock = create_tcp_socket();
    char *ip = get_ip(Test->maxscale_IP);

    if (ip == NULL)
    {
        Test->tprintf("Can't get IP");
        return false;
    }

    struct sockaddr_in *remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
    remote->sin_family = AF_INET;
    int tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));

    if ( tmpres < 0)
    {
        Test->tprintf("Can't set remote->sin_addr.s_addr");
        return false;
    }
    else if (tmpres == 0)
    {
        Test->tprintf("%s is not a valid IP address", ip);
        return false;
    }

    remote->sin_port = htons(4001);

    if (connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
    {
        Test->tprintf("Could not connect");
        return false;
    }

    char *get = cdc_auth_srt((char *) "skysql", (char *) "skysql");
    Test->tprintf("Auth string: %s", get);

    //Send the query to the server
    if (send_so(sock, get) != 0)
    {
        Test->tprintf("Cat't send data to scoket");
        return false;
    }

    char buf1[1024];
    recv(sock, buf1, 1024, 0);

    //Send the query to the server
    if (send_so(sock, reg_str) != 0)
    {
        Test->tprintf("Cat't send data to scoket");
        return false;
    }

    recv(sock, buf1, 1024, 0);

    //Send the query to the server
    if (send_so(sock, req_str) != 0)
    {
        Test->tprintf("Cat't send data to scoket");
        return false;
    }

    Test->stop_timeout();
    int epfd = epoll_create(1);
    static struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev.data.fd = sock;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) < 0)
    {
        Test->tprintf("Error in epoll_ctl! errno = %d, %s", errno, strerror(errno));
        return false;
    }

    epoll_event events[2];
    setnonblocking(sock);

    int inserted_val = 0;
    int ignore_first = 2;

    while (inserted_val < max_inserted_val)
    {
        // wait for something to do...
        Test->tprintf("epoll_wait");
        int nfds = epoll_wait(epfd, &events[0], 1, -1);
        if (nfds < 0)
        {
            Test->tprintf("Error in epoll_wait! errno = %d, %s", errno, strerror(errno));
            return false;
        }

        if (nfds > 0)
        {
            // for each ready socket
            //for(int i = 0; i < nfds; i++)
            //{
            int fd = events[0].data.fd;
            char *json = read_sc(sock);
            Test->tprintf("%s", json);
            //}
            if (ignore_first > 0)
            {
                ignore_first--; // ignoring first reads
                if (ignore_first == 0)
                {
                    // first reads done, starting inserting
                    insert_val = 10;
                    inserted_val = insert_val;
                }
            }
            else
            {
                // trying to check JSON
                long long int x1;
                long long int fl;
                get_x_fl_from_json(json, &x1, &fl);
                Test->tprintf("data received, x1=%lld fl=%lld", x1, fl);

                if (x1 != inserted_val || fl != inserted_val + 100)
                {
                    Test->tprintf("wrong values in JSON");
                }

                inserted_val++;
                insert_val = inserted_val;
            }

            free(json);
        }
        else
        {
            Test->tprintf("waiting");
        }
    }

    free(remote);
    free(ip);
    close(sock);

    return true;
}

static TestConnections *Test;

int main(int argc, char *argv[])
{
    Test = new TestConnections(argc, argv);

    Test->set_timeout(600);
    Test->stop_maxscale();
    Test->ssh_maxscale(true, (char *) "rm -rf /var/lib/maxscale/avro");

    Test->repl->connect();
    execute_query(Test->repl->nodes[0], (char *) "DROP TABLE IF EXISTS t1;");
    Test->repl->close_connections();
    sleep(5);

    Test->binlog_cmd_option = 0;
    Test->start_binlog();

    Test->set_timeout(120);
    Test->stop_maxscale();

    Test->ssh_maxscale(true, (char *) "rm -rf /var/lib/maxscale/avro");

    Test->set_timeout(120);
    Test->start_maxscale();

    Test->set_timeout(60);
    Test->repl->connect();
    create_t1(Test->repl->nodes[0]);
    execute_query(Test->repl->nodes[0], (char *) "INSERT INTO t1 VALUES (111, 222)");
    Test->repl->close_connections();

    Test->stop_timeout();
    Test->tprintf("Waiting 60 seconds for binlogs to be processed.\n"
                  "\n"
                  "+------------------------------------------------------------+\n"
                  "!                                                            !\n"
                  "! This is not OK as we should be able to connect immediately !\n"
                  "!                                                            !\n"
                  "+------------------------------------------------------------+\n");
    sleep(60);

    Test->set_timeout(120);

    pthread_t thread;
    pthread_create( &thread, NULL, query_thread, NULL);

    Test->add_result(!cdc_com(Test), "Failed to execute test");

    exit_flag = true;

    pthread_join(thread, NULL);

    Test->copy_all_logs();
    return Test->global_result;
}

void *query_thread(void *ptr)
{

    Test->repl->connect();

    while (!exit_flag)
    {
        if (insert_val != 0)
        {
            char str[256];
            sprintf(str, "INSERT INTO t1 VALUES (%d, %d)", insert_val, insert_val + 100);
            insert_val = 0;
            execute_query(Test->repl->nodes[0], str);
        }
    }

    Test->repl->close_connections();
}
