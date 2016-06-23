#ifndef MAXINFO_FUNC_H
#define MAXINFO_FUNC_H

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);

/**
* @brief get_maxinfo does request to Maxinfo service and return response JSON
* @param page retrived info name
* @param Test TestConnection object
* @return response from Maxinfo
*/
char * get_maxinfo(char * page, TestConnections* Test);


#endif // MAXINFO_FUNC_H
