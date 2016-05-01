/* 
 * File:   Conn.h
 * Author: Talik Kasozi
 *
 * Created on March 14, 2015, 11:36 PM
 */

#ifndef CONN_H
#define	CONN_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> //close()
#include "TCPStream.h"
#include <arpa/inet.h>  //for sockaddr_in and inet_addr()

class Conn {
public:
   TCPStream * Connect(const char*, int port_num, struct sockaddr_in *serv_addr, int sockfd);
    void resolveHostName(const char* hostname, struct sockaddr_in * serv_addr, int p, int sockfd);
    ~Conn();
private:

};



#endif	/* CONN_H */

