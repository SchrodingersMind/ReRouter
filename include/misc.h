//
// Created by robotknik on 07.09.22.
//

#ifndef REROUTER_MISC_H
#define REROUTER_MISC_H

#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

extern int h_errno;    /* defined by BIND for DNS errors */


#if defined NDEBUG
#define TRACE( format, ... ) do {} while(0)
#else
#define TRACE( format, ... )   printf( "%s::%s(%d) " format, __FILE__, __FUNCTION__,  __LINE__, __VA_ARGS__ )
#endif

// Standard header use \r\n as delimiter between lines
// But it's output looks ugly
// And this function replaces \r\n with \n and output result
void print_headers(char *header) {
    char *orig_header = malloc(strlen(header));
    char *new_header = orig_header;
    while (*header) {
        if (*header != '\r') {
            *new_header = *header;
            new_header++;
        }
        header++;
    }
    printf("%s\n", orig_header);
    free(orig_header);
}

#endif //REROUTER_MISC_H
