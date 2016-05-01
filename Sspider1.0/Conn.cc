/* 
 * File:   Conn.cc
 * Author: Talik Kasozi
 *
 * Created on March 14, 2015, 11:36 PM
 */


#include "Conn.h"
#include "Globals.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

using namespace std; //get rid of this

//Constructor

TCPStream * Conn::Connect(const char* str, int p, struct sockaddr_in * serv_addr, int sockfd) {

    resolveHostName(str, serv_addr, p, sockfd);

    return new TCPStream(sockfd, serv_addr);
}

void Conn::resolveHostName(const char* hostname, struct sockaddr_in * serv_addr, int p, int sockfd) {

    int portno = p, err = -1;
    struct hostent *hp = NULL;


    if ((hp = gethostbyname(hostname)) == NULL) {
        //error("Couldn't get an address.\n");
        std::cout << "Couldn't get an address.Trying again please check connection ..\n";
    }

    while (hp == NULL) {
        hp = gethostbyname(hostname);
    }
    ::memcpy((char*) &serv_addr->sin_addr.s_addr, (char*) hp->h_addr_list[0], hp->h_length);

    serv_addr->sin_port = htons(portno);
    serv_addr->sin_family = AF_INET;

    //while (err < 0) { //try until connection 
    if ((err = ::connect(sockfd, (struct sockaddr *) serv_addr, sizeof (struct sockaddr))) < 0) {
        //error("ERROR connecting");
        std::cout << "ERROR connecting.. " << err << "\n";
    }
    std::cout<<"Connection established.\n";
}

Conn::~Conn() {
}

