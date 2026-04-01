#ifndef __GLOBALS_INCLUDES_SERVER
#define __GLOBALS_INCLUDES_SERVER

#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef enum Response_type_t { FUNC = 0, STRING } response_type_t;
typedef enum http_command_t {
  GET = 0,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE
} http_command_t;

struct http_info {
  char *http_version;
  char *route;
  char *metadata;
  char *content;
  http_command_t request_t;
};

#endif
