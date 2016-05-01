
/* 
 * File:   Parse_TCP.h
 * Author: Talik Kasozi <talik.aziizi@gmail.com>
 *
 * Created on April 30, 2015, 7:20 PM
 */

#ifndef PARSE_TCP_H
#define	PARSE_TCP_H

#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <list>

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

#define NUMLINKS 3
#define NUMSLOTS 500 
#define NUMCONSUMERS 5
#define NUMPRODUCERS 1

class Parse_TCP {

public:

    Parse_TCP();
    Parse_TCP(char *);

    void crawl_page(TidyDoc doc, TidyNode tnod, int indent);
    
    void print_item(std::ostream& stream);

    virtual ~Parse_TCP() {
        //curl_easy_cleanup(curl); //deadlock Alert!!

    };

};

#endif	/* PARSE_TCP_H */



