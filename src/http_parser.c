#include "http_parser.h"
#include "global_includes.h"
#include "router.h"

static const char *headers[] = {"GET",     "HEAD",    "POST",  "PUT", "DELETE",
                                "CONNECT", "OPTIONS", "TRACE", NULL};

static int is_whitespace(char c) {
  const char strip[] = {'\n', '\r', ' ', '\t', 'E'};
  int i = 0;
  while (strip[i] != 'E') {
    if (c == strip[i]) {
      return 1;
    }
    i++;
  }
  return 0;
}

static char *strip_whitespace(char *str) {
  int chars_to_strip = 0;
  size_t s = strlen(str);
  int inds[2] = {0, s - 1};
  for (size_t i = 0; i < s; i++) {
    if (!is_whitespace(str[i])) {
      inds[0] = i;
      break;
    }
  }

  for (size_t i = s - 1; i >= 0; i--) {
    if (!is_whitespace(str[i])) {
      inds[1] = i;
      break;
    }
  }

  if (inds[0] > inds[1] || (inds[0] == 0 && inds[1] == 0)) {
    fprintf(stderr, "Failed to strip whitespace\n");
    return NULL;
  }
  size_t s2 = inds[1] - inds[0] + 2;

  char *buffer = (char *)malloc(sizeof(char) * s2);
  snprintf(buffer, s2, "%s", (str + inds[0]));
  return buffer;
}

static void parse_first_line(struct http_info *inf, const char *line,
                             int line_end) {
  int token_count = 0;
  int token_ind = 0;
  for (int i = 0; i < line_end; i++) {
    if (line[i] == ' ') {
      token_count++;
      if (token_count == 1) {
        int ind = 0;
        int found = 0;
        while (headers[ind] != NULL) {
          if (strncmp(headers[ind], line, strlen(headers[ind])) == 0) {
            found = 1;
            break;
          }
          ind++;
        }
        if (!found) {
          inf->request_t = 255;
        } else {
          inf->request_t = ind;
        }
      } else if (token_count == 2) {
        char *buffer = (char *)malloc(sizeof(char) * (i - token_ind + 1));
        snprintf(buffer, i - token_ind + 1, "%s", (line + token_ind));
        buffer = strip_whitespace(buffer);
        inf->route = buffer;
      }

      token_ind = i;
    }
  }
  char *buffer = (char *)malloc(sizeof(char) * (line_end - token_ind + 1));
  snprintf(buffer, line_end - token_ind + 1, "%s", (line + token_ind));
  buffer = strip_whitespace(buffer);
  inf->http_version = buffer;
}

struct http_info *parse_http_request(const char *req) {
  struct http_info *r = (struct http_info *)malloc(sizeof(struct http_info));
  if (r == NULL) {
    fprintf(stderr, "Failed to allocate mem at parse_http_request");
    return NULL;
  }
  size_t s = strlen(req);
  int line_count = 0;
  int ioll = 0;
  int iols = 0;
  for (size_t i = 0; i < s; i++) {
    if (req[i] == '\r') {
      line_count++;
      if (line_count == 1) {
        parse_first_line(r, req, i);

        iols = i;
      } else if (i - ioll <= 2) {
        // If empty line
        char *buffer = (char *)malloc(sizeof(char) * (i - iols + 1));
        if (!buffer) {
          fprintf(stderr, "Out of memory\n");
        }
        snprintf(buffer, i - iols + 1, "%s", (req + iols));
        buffer = strip_whitespace(buffer);
        r->metadata = buffer;

        iols = i;
      }
      ioll = i;
    }
  }
  // Then copy data into new buffer from start to end
  char *buffer = (char *)malloc(sizeof(char) * (s - iols + 1));
  if (!buffer) {
    fprintf(stderr, "Out of memory\n");
  }
  snprintf(buffer, s - iols + 1, "%s", (req + iols));
  r->content = buffer;

  return r;
}
// req here is the length of the response???
// Why did i write this idek

const char *build_response(const struct http_info *info, const char *resp) {

  if (!resp || !info || !info->http_version) {
    return NULL;
  }

  size_t s = strlen(resp);

  const char *fmt = "%s\r\n"
                    "Server: MyCustomServer/1.0\r\n"
                    "Content-Type: text/html; charset=UTF-8\r\n"
                    "Content-Length: %zu\r\n"
                    "\r\n"
                    "%s";

  int required_len = snprintf(NULL, 0, fmt, info->http_version, s, resp);
  if (required_len < 0) {
    return NULL;
  }

  char *buffer = (char *)malloc(required_len + 1);
  if (buffer == NULL) {
    return NULL;
  }

  snprintf(buffer, required_len + 1, fmt, info->http_version, s, resp);

  return buffer;
}

void free_http_info(struct http_info *info) {
  free(info->http_version);
  free(info->route);
  free(info->metadata);
  free(info->content);
  free(info);
}
