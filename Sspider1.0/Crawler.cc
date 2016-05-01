/* 
 * File:   crawler.cc
 * Author: Talik Kasozi
 *
 * Created on March 14, 2015, 11:36 PM
 */

//#include <iostream>
//#include <cstdio>
//
//
//#include "Conn.h"
//#include "Parse_TCP.h"
//#include <stack>
//
//
//int main(int argc, char **argv) {
//
//    if (argc < 1)
//        std::cout << "There's no link for the WebSpider to crawl." << std::endl;
//
//    Parse_TCP * tidy = new Parse_TCP(argv[1]); //impl functions for delete stack
//    
//    //tidy->print_item(std::cout);
//    // for (int i = 0; i < NUMCONSUMERS + NUMPRODUCERS; i++) 
//      //   std::cout <<i <<"\n";
//    
//
//    return EXIT_SUCCESS;
//}

/* 
 * File:   tester.cc
 * Author: Talik Kasozi <talik.aziizi@gmail.com>
 *
 * Created on May 12, 2015, 12:30 AM
 */

#include <cstdlib>
#include <iostream>

//#include "Sspider.h"
#include "TCPStream.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "Conn.h"
#include <tidy/tidy.h>
#include <tidy/buffio.h>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    int sockfd;

    struct sockaddr_in serv_addr; //mem problem initial

    //    message.
    char message[1024 * 1024] = {0};

    char request[] = "GET /\r\n HTTP/1.1\r\n"
            "Host: www.cnn.com\r\n" //www.w3.org
            "From: talik.aziizi@gmail.com\r\n"
            "User-Agent: webCrawler1.0\r\n"
            "Accept: text/*, text/html, text/html;level=1, */*\r\n"
            "Content-Language:en,en-US\r\n"
            "Cache-Control:no-cache\r\n"
            "Content-Length: 42945\r\n"
            "Connection:closed";

    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("ERROR opening socket");

    memset(&serv_addr, 0, sizeof (serv_addr));

    Conn* connect = new Conn();

    TCPStream *stream = connect->Connect("www.cnn.com", 80, &serv_addr, sockfd); 

    ssize_t bytes_sent = 0, len;
    
    do {
        bytes_sent = stream->send(request);

        do {
            while ((len = recv(sockfd, message, 1024, 0)) > 0)
                std::cout.write(message, len);
            
        } while (len < 0); //while (bytes_received <= 0);

    } while (bytes_sent <= 0);

    close(sockfd);

    return 0;
}

/*std::string CleanHTML(const std::string &html) {

    // Initialize a Tidy document
    TidyDoc tidyDoc = tidyCreate();
    TidyBuffer tidyOutputBuffer = {0};

    // Configure Tidy
    // The flags tell Tidy to output XML and disable showing warnings
    bool configSuccess = tidyOptSetBool(tidyDoc, TidyXmlOut, yes)
            && tidyOptSetBool(tidyDoc, TidyQuiet, yes)
            && tidyOptSetBool(tidyDoc, TidyNumEntities, yes)
            && tidyOptSetBool(tidyDoc, TidyShowWarnings, no);

    int tidyResponseCode = -1;

    // Parse input
    if (configSuccess)
        tidyResponseCode = tidyParseString(tidyDoc, html.c_str());

    // Process HTML
    if (tidyResponseCode >= 0)
        tidyResponseCode = tidyCleanAndRepair(tidyDoc);

    // Output the HTML to our buffer
    if (tidyResponseCode >= 0)
        tidyResponseCode = tidySaveBuffer(tidyDoc, &tidyOutputBuffer);

    // Any errors from Tidy?
    if (tidyResponseCode < 0)
        throw ("Tidy encountered an error while parsing an HTML response. Tidy response code: ");

    // Grab the result from the buffer and then free Tidy's memory
    std::string tidyResult = (char*) tidyOutputBuffer.bp;
    tidyBufFree(&tidyOutputBuffer);
    tidyRelease(tidyDoc);

    return tidyResult;
}*/