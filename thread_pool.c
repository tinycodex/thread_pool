//
// Created by X on 2024/5/30.
//
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "thread_pool.h"


void pool_add(Pool* p, task t) {
    printf("Add Task-begin\n");

    pthread_mutex_lock(&p->lock);
    while (p->t != NULL) {
        pthread_mutex_unlock(&p->lock);
        sched_yield();
        pthread_mutex_lock(&p->lock);
    }

    p->t = t;
    pthread_mutex_unlock(&p->lock);
    printf("Task Added\n");
}




typedef struct {
    int thread_idx;
    Pool * p;
} dispatch_arg;


static void* dispatch(void *arg){
    dispatch_arg* temp = (dispatch_arg*)arg;
    Pool* p = temp->p;
    int thread_idx = temp->thread_idx;

    printf("Thread Created,arg:%p,Pool:%p thread_num:%d\n",arg,p,thread_idx);

    do {
        pthread_mutex_lock(&p->lock);
        while (p->t == NULL) {
            pthread_mutex_unlock(&p->lock);
            sched_yield();
            pthread_mutex_lock(&p->lock);
        }

        task t = p->t;

        // reset
        p->t = NULL;
        pthread_mutex_unlock(&p->lock);

        long int ret;
        ret = (long int )t(NULL);

        if (ret < 0) {
            printf("Exited Type Task,with err:%ld\n",ret);
            break;
        }

    } while (1);

    printf("Thread-%d Exit\n",thread_idx);

    return arg;
}


Pool* pool_create(int num) {
    printf("mem bytes: %ld,%ld\n",num * sizeof(pthread_t),sizeof(Pool));
    Pool* p= malloc( sizeof(Pool));

    pthread_t* tid_s = malloc(num * sizeof(pthread_t));
    p->tid = tid_s;
    p->num = num;

    pthread_mutex_init(&p->lock, NULL);
    pthread_mutex_lock(&p->lock);

    ;
//    pthread_t tid;
    for (int i = 0; i < num; ++i) {
        printf("Creating Thread,No.%d\n",i);
//        pthread_create(&(p->tid[i]),NULL,dispatch, p);
//        pthread_create(&tid,NULL,dispatch, p);
        dispatch_arg *t = malloc(sizeof(dispatch_arg));
        t->thread_idx = i;
        t->p = p;
        pthread_create(&tid_s[i],NULL,dispatch, t);
    }


    pthread_mutex_unlock(&p->lock);
    return p;
}

void pool_destroy(Pool* p){
    dispatch_arg* t;
    for (int i = 0; i < p->num; ++i) {
        pthread_join(p->tid[i],((void**)&t));
        printf("args:%p\n",t);
        free(t);
    }

    pthread_mutex_destroy(&p->lock);
    free(p->tid);
    free(p);
}




