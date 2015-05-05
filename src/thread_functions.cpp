#include "thread_functions.h"
#include <pthread.h>
#include <iostream>
#include <cstdlib>

void create_thread(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	int rc = pthread_create(thread, attr, start_routine, arg);
	if (rc)
	{
		std::cerr << "Could not create thread." << std::endl;
		exit(1);
	}
}
