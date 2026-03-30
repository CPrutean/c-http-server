#ifndef __HTTP_PARSER_SERVER
#define __HTTP_PARSER_SERVER
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct http_info *parse_http_request(const char *req);
char *post_response(const struct http_info *info, const char *resp);
void free_http_info(struct http_info *info);
#endif
