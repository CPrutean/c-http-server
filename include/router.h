#ifndef __ROUTER_SERVER
#define __ROUTER_SERVER
#include "global_includes.h"

// If you define a callback it is YOUR JOB to parse the command
typedef char *(*http_callback)(char *, http_command_t);
void add_route(char *route, void *ptr, response_type_t t);
response_type_t get_route_type(char *route);
void update_route(char *route, void *ptr, response_type_t t);
char *route_command(char *str, http_command_t command);
void dump_routes(void);
void init_routes(void);
void close_routes(void);

#endif
