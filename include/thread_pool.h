#ifndef __THREAD_POOL_SERVER
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_threads(int num_threads);
void dispatch_connection(int sockfd);

#endif
