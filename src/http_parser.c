#include "http_parser.h"
#include "router.h"

static const char *headers[] = {"GET",     "HEAD",    "POST",  "PUT", "DELETE",
                                "CONNECT", "OPTIONS", "TRACE", NULL};
// For help with parsing
static const char *http_response_bodies[] = {
    "Date:",     "Server:", "Content-Type:", "Content-Length:", "Connection:",
    "Location:", NULL};

static const char **sections[] = {headers, http_response_bodies, NULL};

static int check_in_curr_sec(int sc, const char *line) {
  int i = 0;
  int found = 0;
  while (sections[sc][i] != NULL) {
    if (strncmp(line, sections[sc][i], strlen(sections[sc][i])) == 0) {
      found = 1;
      break;
    }
    i++;
  }
  return found;
}

static void parse_first_line(int sc_ind[2], const char *req,
                             struct http_info *info) {
  int token_count = 0;
  int token_inds[2] = {0, 0};
  int l = sc_ind[1] - sc_ind[0];
  int tl;
  char *buffer;
  for (int i = 0; i < l; i++) {
    if (req[i] == ' ') {
      token_inds[0] = token_inds[1];
      token_inds[1] = i;
      token_count++;
      if (token_count != 1) {
        tl = token_inds[1] - token_inds[0] + 1;
        buffer = (char *)malloc(sizeof(char) * tl);
        snprintf(buffer, tl, "%s", (req + token_inds[0]));
        buffer[tl - 1] = '\0';
      }
    }

    // Command type
    if (token_count == 1) {
      int j = 0;
      while (headers[j] != NULL) {
        if (strncmp(headers[j], req, strlen(headers[j])) == 0) {
          info->request_t = (http_command_t)j;
          break;
        }
        j++;
      }
    } else if (token_count == 2) {
      info->route = buffer;
    } else if (token_count == 3) {
      info->http_version = buffer;
    } else {
      fprintf(stderr, "More than 3 tokens in initial line");
    }
  }
}

static void print_into_section(int section_count, int sc_ind[2],
                               const char *req, struct http_info *info) {
  if (req == NULL || info == NULL) {
    fprintf(stderr, "print_into_section");
    return;
  }

  size_t s = sc_ind[1] - sc_ind[0] + 1;
  char *buff = (char *)malloc(sizeof(char) * s);
  if (buff == NULL) {
    fprintf(stderr, "Failed to allocate thread memory");
    return;
  }

  switch (section_count) {
  // First initial header containing root, version and command
  case (0):
    parse_first_line(sc_ind, req, info);
    break;
  // header for our metadata
  case (1):
    snprintf(buff, s, "%s", req);
    buff[s - 1] = '\0';
    info->metadata = buff;
    break;
  // For the content part of the header
  case (2):
    snprintf(buff, s, "%s", req);
    buff[s - 1] = '\0';
    info->content = buff;
    break;
  default:
    break;
  };
}

struct http_info *parse_http_request(const char *req) {
  struct http_info *r = (struct http_info *)malloc(sizeof(struct http_info));
  if (r == NULL) {
    fprintf(stderr, "Failed to allocate mem at parse_http_request");
  }
  size_t slen = strlen(req);

  int section_count = 0;
  int end_of_section[2] = {0, 0};

  for (size_t i = 0; i < slen - 1; i++) {
    if (req[i] == '\r' && check_in_curr_sec(section_count, &req[i])) {
      section_count++;
      end_of_section[0] = end_of_section[1];
      end_of_section[1] = i;
      print_into_section(section_count, end_of_section, req, r);
    }
  }

  return r;
}

// req here is the length of the response???
// Why did i write this idek
char *post_response(const struct http_info *info, const char *req,
                    int is_string) {

  size_t s = strlen(req);
  if (is_string) {
    size_t len = 0;
    char *buffer = (char *)malloc(sizeof(char) * s * 2);
    len += snprintf(buffer, s * 2, "%s", info->http_version);
    len += snprintf((buffer + len), s * 2, "%s", "\r\n");
    len += snprintf((buffer + len), s * 2, "%s", "Server: MyCustomServer/1.0");
    len += snprintf((buffer + len), s * 2, "%s", "\r\n");
    len += snprintf((buffer + len), s * 2, "%s",
                    "Content-Type: text/html; charset=UTF-8");
    len += snprintf((buffer + len), s * 2, "%s", "\r\n");
    len += snprintf((buffer + len), s * 2, "%s %ld", "Content-Length: ", s);
    len += snprintf((buffer + len), s * 2, "%s", "\r\n");
    len += snprintf((buffer + len), s * 2, "%s", "req");
    return buffer;
  } else {
    // Function will handle its own http headers due to potential custom nature
    return req;
  }
}

void free_http_info(struct http_info *info) {
  free(info->http_version);
  free(info->route);
  free(info->metadata);
  free(info->content);
  free(info);
}
