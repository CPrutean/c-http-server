#ifndef __THREAD_POOL_SERVER
#define __THREAD_POOL_SERVER

#include "global_includes.h"
#include <pthread.h>

// Stops worker threads
void stop_worker_threads(void);
// cleans up worker threads
void clean_workers(void);
void init_threads(int num_threads, int sockfd);
void dispatch_connection(int sockfd);

#endif
