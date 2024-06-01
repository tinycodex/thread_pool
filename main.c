//
// Created by X on 2024/5/31.
//

#include <stdio.h>
#include "thread_pool.h"

void* thread_func(void * t) {
    printf("start task process\n");
    printf("task process end\n");


    return (void *)-1;
}

int main(void) {
    Pool* p = pool_create(10);

    Task t;
    t.args = NULL;
    t.entry = thread_func;

    pool_add_task(p, t);
    pool_add_task(p, t);
    pool_add_task(p, t);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);

//    sleep(10);
    pool_destroy(p);

    return 0;
}

