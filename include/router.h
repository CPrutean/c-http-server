#ifndef __ROUTER_SERVER
#define __ROUTER_SERVER

#include "http_parser.h"
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef enum Response_type_t { FUNC, STRING } response_type_t;

// If you define a callback it is YOUR JOB to parse the command
typedef char *(*http_callback)(char *, http_command_t);
void add_route(char *route, void *ptr, response_type_t t);
void update_route(char *route, void *ptr, response_type_t t);
char *route_command(char *str, http_command_t command);
void dump_routes(void);
void init_routes(void);
void close_routes(void);

#endif
