//
// Created by X on 2024/5/30.
//
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "thread_pool.h"

static pthread_mutex_t queue_manager_lock = PTHREAD_MUTEX_INITIALIZER;

static int queue_is_null(taskQueue *q) {
    pthread_mutex_lock(&queue_manager_lock);
    if (q == NULL) {
        pthread_mutex_unlock(&queue_manager_lock);
        return 1;
    }
    pthread_mutex_unlock(&queue_manager_lock);

    return 0;
}


taskQueue *queueCreate(size_t cap) {
    size_t memByteSize = sizeof(Task) * cap + sizeof(taskQueue);
    taskQueue *q = malloc(memByteSize);

    if (q == NULL) {
        perror("queue_init malloc()");
        return NULL;
    }

    memset(q, 0, memByteSize);
    pthread_mutex_init(&q->lock, NULL);
    q->cap = cap;

    return q;
}

void enQueue(taskQueue *q, Task task) {
    // 检查队列资源是否还存在
    if (queue_is_null(q))
        return;

    if (q->destroyed == 1) {
        return;
    }

    // 对队列的读写操作进行上锁
    pthread_mutex_lock(&q->lock);
    q->write_num += 1;

    do {
        while (q->len >= q->cap && q->destroyed == 0) {
            pthread_mutex_unlock(&q->lock);
            sched_yield();
            pthread_mutex_lock(&q->lock);
        }

        if (q->destroyed != 0) {
            break;
        }

        Task *t = &q->task[q->rear];
        t->args = task.args;
        t->entry = task.entry;

        q->rear = (q->rear + 1) % q->cap;
        q->len += 1;

    } while (0);

    q->write_num -= 1;
    //defer op
    pthread_mutex_unlock(&q->lock);

    return;
}

Task deQueue(taskQueue *q) {
    Task task = {0};

    // 检查队列资源是否还存在
    if (queue_is_null(q))
        return task;

    if (q->destroyed == 1) {
        return task;
    }

    // 对队列的读操作进行上锁
    pthread_mutex_lock(&q->lock);

    q->read_num += 1;
    do {
        while (q->len == 0 && q->destroyed == 0) {
            pthread_mutex_unlock(&q->lock);
            sched_yield();
            pthread_mutex_lock(&q->lock);
        }

        if (0 != q->destroyed) {
            break;
        }

        task = q->task[q->front];
        q->front = (q->front + 1) % q->cap;
        q->len -= 1;
    } while (0);

    q->read_num -= 1;

    //defer op
    pthread_mutex_unlock(&q->lock);

    return task;
}


void destroyQueue(taskQueue *q) {
    // 检查队列资源是否还存在
    if (queue_is_null(q))
        return;

    //释放阻塞在队列读写上的线程
    q->destroyed = 1;

    //释放队列读写锁,等待所有读写进程都正常推出
    pthread_mutex_lock(&q->lock);

    while(q->read_num>0 || q->write_num > 0) {
        pthread_mutex_unlock(&q->lock);
        sched_yield();
        pthread_mutex_lock(&q->lock);
    }

    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);
    free(q);
}

void pool_add_task(Pool *p, Task t) {
    printf("Add Task-begin\n");
    enQueue(p->queue, t);
    printf("Task Added\n");
}


typedef struct {
    int thread_idx;
    Pool *p;
} dispatch_arg;


static void *work_thread(void *arg) {
    dispatch_arg *temp = (dispatch_arg *) arg;
    Pool *p = temp->p;
    int thread_idx = temp->thread_idx;

    printf("[Thread-%d] Created,arg:%p,Pool:%p\n", thread_idx, arg, p);

    while (!p->terminate) {
        Task task = deQueue(p->queue);

        printf("[No.Thread-%d] ", thread_idx);

        if (task.entry != NULL)
            task.entry(task.args);
        //TODO: 调整线程数量，空闲回收，忙绿新增
        //idle_exit、too_busy_add_more

    }

    // 线程资源回收
    // pthread_detach(pthread_self());

    printf("[Thread-%d] exit\n", thread_idx);

    return arg;
}


Pool *pool_create(int num) {
    printf("mem bytes: %ld,%ld\n", num * sizeof(pthread_t), sizeof(Pool));

    // 初始化线程池
    size_t memByteSize = sizeof(Pool) + num * sizeof(pthread_t);

    Pool *p = malloc(memByteSize);
    if (p == NULL) {
        perror("pool_create(),malloc()");
        return NULL;
    }

    memset(p, 0, memByteSize);
    pthread_mutex_init(&p->lock, NULL);
    p->num = num;

    taskQueue *q = queueCreate(100);
    if (q == NULL) {
        return NULL;
    }

    p->queue = q;

    for (int i = 0; i < num; ++i) {
        printf("Creating Thread,No.%d\n", i);
        dispatch_arg *t = malloc(sizeof(dispatch_arg));
        t->thread_idx = i;
        t->p = p;
        pthread_create(&p->tid[i], NULL, work_thread, t);
    }

    return p;
}

// 资源应该如何安全的释放？
static pthread_mutex_t pool_lock = PTHREAD_MUTEX_INITIALIZER;

void pool_destroy(Pool *p) {
    pthread_mutex_lock(&pool_lock);
    if (p == NULL) {
        pthread_mutex_unlock(&pool_lock);
        return;
    }
    pthread_mutex_unlock(&pool_lock);

    // 释放工作线程,放在下面会有一些无用的空转
    p->terminate = 1;

    dispatch_arg *arg;
    // 释放队列
    destroyQueue(p->queue);

//    // 释放工作线程
//    p->terminate = 1;

    // 线程资源回收
    for (int i = 0; i < p->num; ++i) {
        pthread_join(p->tid[i], (void **) &arg);
        printf("Thread-%d was recycled\n", arg->thread_idx);
    }

    pthread_mutex_destroy(&p->lock);

    // 释放线程池管理者内存资源
    free(p);
    p = NULL;
}




