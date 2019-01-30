// Last Update:2019-01-30 16:29:55
/**
 * @file queue.h
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-09-12
 */

#ifndef QUEUE_INTERNAL_H
#define QUEUE_INTERNAL_H

#include <pthread.h>

typedef struct Element {
    struct Element *next;
    void *data;
    int size;
} Element;

typedef struct Queue {
    Element             *pIn;
    Element             *pOut;
    Element             *pLast;
    int                 size;
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;

    int (*enqueue)( struct Queue *q, void *data, int size );
    int (*dequeue)( struct Queue *q, void *data, int *outSize );
    int (*getSize)( struct Queue *q );
} Queue;

Queue* NewQueue();

#endif  /*QUEUE_H*/
