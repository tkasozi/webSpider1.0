/* 
 * File:   TCPStream.cc
 * Author: Talik Kasozi
 *
 * Created on March 16, 2015, 11:11 PM
 */

#include "TCPStream.h"
#include  <arpa/inet.h>
#include <unistd.h> 

TCPStream::TCPStream(int sd, struct sockaddr_in* addr)
: sock_id(sd), address(addr) {
    char ip[50];

    inet_ntop(PF_INET, (struct in_addr*) &(address->sin_addr.s_addr), ip, sizeof (ip) - 1);

    m_peerIP = ip;
    m_peerPort = ntohs(address->sin_port);
}

ssize_t TCPStream::receive(char* request) {

    return ::read(sock_id, request, 1024 * 1024);
}

ssize_t TCPStream::send(const char* request) {
    std::cout << "request sent.\n";
    return write(sock_id, request, strlen(request));
    ;
}

std::string TCPStream::getPeerIP() {
    return m_peerIP;
}

int TCPStream::getPeerPort() {
    return m_peerPort;
}

TCPStream::~TCPStream() {
    close(sock_id);
}