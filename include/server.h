#ifndef __HTTP_SERVER
#define __HTTP_SERVER
#include "global_includes.h"
#include "thread_pool.h"

void start_server(int sockfd);
void set_max_connections(int connections);
int get_max_connections(void);
int check_connections(int sockfd);
void server_loop(int sockfd);

void generate_hints(struct addrinfo *hints);
int create_server_socket(int port);

#endif
