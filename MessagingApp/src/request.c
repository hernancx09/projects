/* request.c: Request structure */

#include "mq/request.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Create Request structure.
 * @param   method      Request method string.
 * @param   uri         Request uri string.
 * @param   body        Request body string.
 * @return  Newly allocated Request structure.
 */
Request * request_create(const char *method, const char *uri, const char *body) {
    Request *new_request = malloc(sizeof(Request));
    if (!new_request) return NULL; // malloc failed

    if(method) new_request->method = strdup(method);
    else new_request->method = NULL;

    if(uri) new_request->uri = strdup(uri);
    else new_request->uri = NULL;

    if(body) new_request->body = strdup(body);
    else new_request->body = NULL;
    return new_request;
}

/**
 * Delete Request structure.
 * @param   r           Request structure.
 */
void request_delete(Request *r) {
    if(!r) return; // make sure r isnt null

    if (r->method) free(r->method);
    if (r->uri) free(r->uri);
    if (r->body) free(r->body);
    free(r);

}

/**
 * Write HTTP Request to stream:
 *  
 *  $METHOD $URI HTTP/1.0\r\n
 *  Content-Length: Length($BODY)\r\n
 *  \r\n
 *  $BODY
 *      
 * @param   r           Request structure.
 * @param   fs          Socket file stream.
 */
void request_write(Request *r, FILE *fs) {


    fprintf(fs, "%s %s HTTP/1.0\r\n", r->method, r->uri);
    if(r->body) fprintf(fs, "Content-Length: %ld\r\n", strlen(r->body));
    // the if statements make it so that it gives correct output when the requesst does not have an actual body
    fprintf(fs, "\r\n");                
    if(r->body) fprintf(fs, "%s", r->body);                 

}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */ 
