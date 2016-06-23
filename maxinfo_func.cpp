#include <my_config.h>
#include <iostream>
#include <unistd.h>
#include "testconnections.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

using namespace std;

#define PORT 8080
#define USERAGENT "HTMLGET 1.1"

int create_tcp_socket()
{
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("Can't create TCP socket");
        return 0;
    }
    return sock;
}

char *get_ip(char *host)
{
    struct hostent *hent;
    int iplen = 15; //XXX.XXX.XXX.XXX
    char *ip = (char *)malloc(iplen+1);
    memset(ip, 0, iplen+1);
    if((hent = gethostbyname(host)) == NULL)
    {
        herror("Can't get IP");
        return NULL;
    }
    if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
    {
        perror("Can't resolve host");
        return NULL;
    }
    return ip;
}

char *build_get_query(char *host, char *page)
{
    char *query;
    char *getpage = page;
    char *tpl = (char *) "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
    if(getpage[0] == '/'){
        getpage = getpage + 1;
        fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
    }
    // -5 is to consider the %s %s %s in tpl and the ending \0
    query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
    sprintf(query, tpl, getpage, host, USERAGENT);
    return query;
}

char * get_maxinfo(char * page, TestConnections* Test)
{
    struct sockaddr_in *remote;
    int sock;
    int tmpres;
    char *ip;
    char *get;
    char buf[BUFSIZ+1];

    sock = create_tcp_socket();
    ip = get_ip(Test->maxscale_IP);
    if (ip != NULL)
    {
        Test->add_result(1, "Can't get IP\n");
        return NULL;
    }
    remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
    remote->sin_family = AF_INET;
    tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
    if( tmpres < 0)
    {
        Test->add_result(1, "Can't set remote->sin_addr.s_addr\n");
        return NULL;
    } else if (tmpres == 0)
    {
        Test->add_result(1, "%s is not a valid IP address\n", ip);
        return NULL;
    }
    remote->sin_port = htons(PORT);

    if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
        Test->add_result(1, "Could not connect\n");
        return NULL;
    }
    get = build_get_query(Test->maxscale_IP, page);
    //Test->tprintf("Query is:\n<<START>>\n%s<<END>>\n", get);

    //Send the query to the server
    int sent = 0;
    while(sent < strlen(get))
    {
        tmpres = send(sock, get+sent, strlen(get)-sent, 0);
        if(tmpres == -1){
            Test->add_result(1, "Can't send query\n");
            return NULL;
        }
        sent += tmpres;
    }
    //now it is time to receive the page
    memset(buf, 0, sizeof(buf));

    char* result = (char*)calloc(BUFSIZ, sizeof(char));
    size_t rsize = sizeof(buf);
    while((tmpres = recv(sock, buf, BUFSIZ, MSG_WAITALL)) > 0){
        result = (char*)realloc(result, tmpres + rsize);
        rsize += tmpres;
        strcat(result, buf);
        memset(buf, 0, tmpres);
    }
    if(tmpres < 0)
    {
        Test->add_result(1, "Error receiving data\n");
        return NULL;
    }

    free(get);
    free(remote);
    free(ip);
    close(sock);

    char * content = strstr(result, "[");
    if (content == NULL)
    {
        Test->add_result(1, "Content not found\n");
        free(result);
        return NULL;
    }

    char * ret_content = (char*) calloc(strlen(content)+1, sizeof(char));
    mempcpy(ret_content, content, strlen(content));
    free(result);

    return(ret_content);
    //return(result);
}
