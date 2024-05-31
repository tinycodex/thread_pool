//
// Created by X on 2024/5/30.
//

#ifndef THREAD_POLL_THREAD_POOL_H
#define THREAD_POLL_THREAD_POOL_H

#include <pthread.h>

typedef void* (*task)(void*);

typedef struct {
    task t;
    pthread_mutex_t lock;
    int num;
    pthread_t tid[0];
} Pool;

void pool_add(Pool* p, task t);
Pool* pool_create(int num);
void pool_destroy(Pool* p);

#endif //THREAD_POLL_THREAD_POOL_H
