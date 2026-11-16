/* client.c: Message Queue Client */

#include "mq/client.h"
#include "mq/logging.h"
#include "mq/socket.h"
#include "mq/string.h"

/* Internal Constants */

#define SENTINEL "SHUTDOWN"

/* Internal Prototypes */

void * mq_pusher(void *);
void * mq_puller(void *);
pthread_mutex_t shutdown_mutex = PTHREAD_MUTEX_INITIALIZER;


/* External Functions */

/**
 * Create Message Queue withs specified name, host, and port.
 * @param   name        Name of client's queue.
 * @param   host        Address of server.
 * @param   port        Port of server.
 * @return  Newly allocated Message Queue structure.
 */
MessageQueue * mq_create(const char *name, const char *host, const char *port) { // prolly wworks
    MessageQueue *new = malloc(sizeof(MessageQueue));
    if (!new) return NULL;
    
    strcpy(new->name, name);
    strcpy(new->host, host);
    strcpy(new->port, port);
    new->shutdown = false;
    new->incoming = queue_create();
    new->outgoing = queue_create();
    
    return new;
}

/**
 * Delete Message Queue structure (and internal resources).
 * @param   mq      Message Queue structure.
 */
void mq_delete(MessageQueue *mq) { // prolly yworks
    queue_delete(mq->incoming);
    queue_delete(mq->outgoing);
    free(mq);
    pthread_mutex_destroy(&shutdown_mutex);
}

/**
 * Publish one message to topic (by placing new Request in outgoing queue).
 * @param   mq      Message Queue structure.
 * @param   topic   Topic to publish to.
 * @param   body    Message body to publish.
 */
void mq_publish(MessageQueue *mq, const char *topic, const char *body) { // prolly works
    // PUT /topic/$TOPIC
    char uri[BUFSIZ];
    sprintf(uri, "/topic/%s", topic);
    Request *request = request_create("PUT", uri, body);
    queue_push(mq->outgoing, request);
}

/**
 * Retrieve one message (by taking Request from incoming queue).
 * @param   mq      Message Queue structure.
 * @return  Newly allocated message body (must be freed).
 */
char * mq_retrieve(MessageQueue *mq) {

    Request *request = queue_pop(mq->incoming); 
    char *body = request->body;
    request->body = NULL; // make sure body is null before deleting so that the new body made doesnt get its data yoinked 

    if (body && streq(body, SENTINEL)){ // make sure body exists
        free(body);
        body = NULL; // if body exists and the body is the sentinel set the body to null 
    }
    request_delete(request);
    return body;
}

/**
 * Subscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to subscribe to.
 **/
void mq_subscribe(MessageQueue *mq, const char *topic) { 
    // Put /subscription/$Queue/$topic
    char uri[BUFSIZ];
    sprintf(uri, "/subscription/%s/%s", mq->name, topic);
    Request *request = request_create("PUT", uri, NULL);
    queue_push(mq->outgoing, request);
}

/**
 * Unubscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to unsubscribe from.
 **/
void mq_unsubscribe(MessageQueue *mq, const char *topic) { 
    // DELETE /subscriptoion/$Queue/$topic
    char uri[BUFSIZ];
    sprintf(uri, "/subscription/%s/%s", mq->name, topic);
    Request *request = request_create("DELETE", uri, NULL);
    queue_push(mq->outgoing, request);
}

/**
 * Start running the background threads:
 *  1. First thread should continuously send requests from outgoing queue.
 *  2. Second thread should continuously receive reqeusts to incoming queue.
 * @param   mq      Message Queue structure.
 */
void mq_start(MessageQueue *mq) { 
    mq_subscribe(mq, SENTINEL); // make sure threads will look at the sentinel  
    thread_create(&mq->push_thread, NULL, mq_pusher, mq);
    thread_create(&mq->pull_thread, NULL, mq_puller, mq);

}

/**
 * Stop the message queue client by setting shutdown attribute and sending
 * sentinel messages
 * @param   mq      Message Queue structure.
 */
void mq_stop(MessageQueue *mq) {
    mq_publish(mq, SENTINEL, SENTINEL);
    pthread_mutex_lock(&shutdown_mutex); // lock
    mq->shutdown = true;
    pthread_mutex_unlock(&shutdown_mutex); // unlock
    thread_join(mq->push_thread, NULL);
    thread_join(mq->pull_thread,NULL);

}

/**
 * Returns whether or not the message queue should be shutdown.
 * @param   mq      Message Queue structure.
 */
bool mq_shutdown(MessageQueue *mq) { 

    bool shutdown_status; 
    pthread_mutex_lock(&shutdown_mutex); // lock
    shutdown_status = mq->shutdown;
    pthread_mutex_unlock(&shutdown_mutex); // unlock
    return shutdown_status; 
}

/* Internal Functions */

/**
 * Pusher thread takes messages from outgoing queue and sends them to server.
 **/
void * mq_pusher(void *arg) { 
    MessageQueue *mq = (MessageQueue *)arg; 
    FILE *fs;

    char buffer[BUFSIZ];
    while (!mq_shutdown(mq)) { 

        fs  = socket_connect(mq->host, mq->port); // connecting in the loop 
        if (!fs) return NULL;
        Request *request = queue_pop(mq->outgoing);
        if (streq(request->method, SENTINEL)) {
            request_delete(request);
        fclose(fs);
            break;
        }
        request_write(request, fs);
        while (fgets(buffer, BUFSIZ, fs)){
                buffer[BUFSIZ  -1] = '\0'; // make last char of buffer be a null 
            }
     
        fclose(fs);
        request_delete(request);
    }

    return NULL;
}

/**
 * Puller thread requests new messages from server and then puts them in
 * incoming queue.
 **/
void * mq_puller(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;
    FILE *fs;
    size_t length =-1;

    char uri[BUFSIZ] = "\0";
    char body[BUFSIZ] = "\0";
    while (!mq_shutdown(mq)) {
        fs = socket_connect(mq->host, mq->port);
        if (!fs) return NULL;
        // create GET /queue/name
        sprintf(uri, "/queue/%s", mq->name);
        Request *request = request_create("GET", uri, NULL);
        request_write(request, fs); // write that we want to get a request from a topic and get its body 
             // READ 
        if (fgets(body, BUFSIZ, fs) && strstr(body, "200 OK")) { // make sure the rquest was fine
            while(fgets(body, BUFSIZ, fs) && !streq(body, "\r\n")){ 
                sscanf(body, "Content-Length: %ld", &length); // yoink the content length 
            }    

            request->body = calloc(length + 1, sizeof(char));
            fread(request->body, 1, length, fs); // thing read after the content length is the body 
            // we want content length to be added so remake the request  
            Request *new = request_create(request->method, request->uri, request->body);
            request_delete(request);

            queue_push(mq->incoming, new);
        }
        else request_delete(request); //  if the response wasnt '200 ok' then delete the request
        fclose(fs);
    }

    return NULL;
}
// iffy functions left are puller > pusher > retriever > stop

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
