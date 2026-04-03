#ifndef __HTTP_PARSER_SERVER
#define __HTTP_PARSER_SERVER
#include "global_includes.h"

struct http_info *parse_http_request(const char *req);

const char *build_response(const struct http_info *info, const char *resp);

void free_http_info(struct http_info *info);
#endif
