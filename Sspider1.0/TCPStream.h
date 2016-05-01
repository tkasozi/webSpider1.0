/* 
 * File:   TCPStream.h
 * Author: Talik Kasozi
 * 
 * credit to http://vichargrave.com/network-programming-design-patterns-in-c/
 * Created on March 16, 2015, 11:11 PM
 */

#ifndef TCPSTREAM_H
#define	TCPSTREAM_H

#include <iostream>
#include <string.h>

class TCPStream{
    int sock_id;
    std::string m_peerIP;
    int m_peerPort;
    struct sockaddr_in* address;
    
public:
     friend class Conn;
     
     ssize_t receive(char*  buffer); //std::string*
     ssize_t send(const char* buffer);
     
     std::string getPeerIP();
     int getPeerPort();
     
     ~TCPStream();
private:
    TCPStream();
    TCPStream(int sd,struct sockaddr_in* address);
    TCPStream(const TCPStream &stream);
};

#endif	/* TCPSTREAM_H */

