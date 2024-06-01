//
// Created by X on 2024/5/30.
//

#ifndef THREAD_POLL_THREAD_POOL_H
#define THREAD_POLL_THREAD_POOL_H

#include <pthread.h>

typedef struct {
    void* (*entry)(void* args);
    void* args;
} Task;

typedef struct {
    pthread_mutex_t lock;     //对队列数据读写进行并发控制
    int read_num,write_num;   //读写线程的数量
    int destroyed;            //队列销毁回收信号，不再进行读写
    size_t front,rear;        //队列头尾
    size_t cap,len;           //队列容量、当前元素数量
    Task task[0];             //队列元素存储
} taskQueue;

taskQueue* queueCreate(size_t cap);



typedef struct {
    pthread_mutex_t lock;
    int terminate;

    taskQueue* queue;
    int num;
    pthread_t tid[0];
} Pool;

Pool* pool_create(int num);
void pool_destroy(Pool* p);


void pool_add_task(Pool* p, Task t);


#endif //THREAD_POLL_THREAD_POOL_H
