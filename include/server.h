#ifndef __HTTP_SERVER
#define __HTTP_SERVER

#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

void start_server(void);
void stop_server(void);
void set_max_connections(int connections);
int get_max_connections(void);
int check_connections(int sockfd);
void server_loop(int sockfd);

void generate_hints(struct addrinfo *hints);
int create_server_socket(int port);

#endif
