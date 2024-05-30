//
// Created by X on 2024/5/31.
//

#include <stdio.h>
#include "thread_pool.h"

void* thread_func(void * t) {
    printf("Hello from thread\n");
    return (void *)-1;
}

int main(void) {
    Pool* p = pool_create(3);

    pool_add(p, thread_func);
    pool_add(p, thread_func);
    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);
//    pool_add(p, thread_func);

    pool_destroy(p);

    return 0;
}

