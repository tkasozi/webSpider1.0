/* 
 * File:   Globals.h
 * Author: apple
 *
 * Created on March 15, 2015, 12:33 AM
 */

#ifndef GLOBALS_H
#define	GLOBALS_H

#include <cstdio>
#include <cstdlib>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


#endif	/* GLOBALS_H */

