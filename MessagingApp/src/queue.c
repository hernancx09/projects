/* queue.c: Concurrent Queue of Requests */

#include "mq/queue.h"

/**
 * Create queue structure.
 * @return  Newly allocated queue structure.
 */
Queue * queue_create() {
    Queue *new_queue = malloc(sizeof(Queue));
    if (!new_queue) return NULL;
    
    new_queue->size = 0;
    new_queue->head = NULL; 
    new_queue->tail  = NULL;

    sem_init(&new_queue->lock, 0, 1); // lock is open to be yoinked when it equals 1
    sem_init(&new_queue->produced,0, 0); // 0 things produced at the begining 
    return new_queue;
}

/**
 * Delete queue structure.
 * @param   q       Queue structure.
 */
void queue_delete(Queue *q) {
    Request *cur = q->head;
    Request *tmp = NULL;
    while(1){
        if(!cur) break;
        tmp = cur;
        cur = tmp->next;
        request_delete(tmp);
    }
    free(q);
}

/**
 * Push request to the back of queue.
 * @param   q       Queue structure.
 * @param   r       Request structure.
 */
void queue_push(Queue *q, Request *r) {
    
    sem_wait(&q->lock);
    if (!q->head) q->head = r;
    else q->tail->next= r;
    q->tail =r;
    r->next = NULL;
    q->size +=1;

    sem_post(&q->produced); // produced +=1 kinda
    sem_post(&q->lock);
}

/**
 * Pop request to the front of queue (block until there is something to return).
 * @param   q       Queue structure.
 * @return  Request structure.
 */
Request * queue_pop(Queue *q) {
    sem_wait(&q->produced); // wait if produced <0 otherwise produced -=1 ish
    sem_wait(&q->lock);
    
    Request *head = q->head;
    q->size -=1;
    q->head = head->next;
    sem_post(&q->lock);
    return head;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
