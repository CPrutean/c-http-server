#ifndef __THREAD_POOL_SERVER
#define __THREAD_POOL_SERVER
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stops worker threads
void stop_worker_threads(void);
// cleans up worker threads
void clean_workers(void);
void init_threads(int num_threads);
void dispatch_connection(int sockfd);

#endif
