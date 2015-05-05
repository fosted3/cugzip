#ifndef thread_functions_h_
#define thread_functions_h_

#include <pthread.h>

void create_thread(pthread_t*, const pthread_attr_t*, void* (*)(void*) , void*);

#endif
