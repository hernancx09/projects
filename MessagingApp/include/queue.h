/* queue.h: Concurrent Queue of Requests */

#ifndef QUEUE_H
#define QUEUE_H

#include "mq/request.h"
#include "mq/thread.h"
#include <semaphore.h>

/* Structures */

typedef struct Queue Queue;
struct Queue {
    Request *head;
    Request *tail;
    size_t   size;

    sem_t lock;
    sem_t produced;
    // do we need aa consumed? i dont think so tbh 

    // TODO: Add any necessary thread and synchronization primitives
};

/* Functions */

Queue *	    queue_create();
void        queue_delete(Queue *q);

void	    queue_push(Queue *q, Request *r);
Request *   queue_pop(Queue *q);

#endif

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
