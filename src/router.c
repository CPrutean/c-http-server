#include "router.h"
#include "global_includes.h"

static char **routes;
static void **callbacks;
static response_type_t *callback_types;
static int num_routes;

static int get_route_ind(char *route) {
  if (route == NULL) {
    return -1;
  }

  for (int i = 0; i < num_routes; i++) {
    if (strncmp(routes[i], route, strlen(routes[i])) == 0) {
      return i;
    }
  }

  return -1;
}
void init_routes(void) {
  routes = (char **)malloc(sizeof(char *) * 2);
  callback_types = (response_type_t *)malloc(sizeof(response_type_t) * 2);
  for (int i = 0; i < 2; i++) {
    routes[i] = (char *)malloc(sizeof(char) * 6);
  }
  callbacks = (void *)malloc(sizeof(void *) * 2);
  snprintf(routes[0], 2, "%s", "/");
  routes[0][1] = '\0';
  callbacks[0] = NULL;

  snprintf(routes[1], 6, "%s", "/root");
  routes[1][5] = '\0';
  callbacks[1] = NULL;

  num_routes = 2;
}

void add_route(char *route, void *ptr, response_type_t t) {
  int i = get_route_ind(route);
  if (i != -1) {
    update_route(route, ptr, t);
  } else {
    char **new_r = (char **)malloc(sizeof(char *) * (num_routes + 1));
    if (new_r == NULL) {
      fprintf(stderr, "Failed to allocate memory for new routes");
      return;
    }
    size_t s = strlen(route);
    memcpy(new_r, routes, sizeof(char *) * (s + 1));
    free(routes);
    routes = new_r;

    char *new_route = (char *)malloc(sizeof(char) * (s + 1));
    if (new_route == NULL) {
      fprintf(stderr, "Failed to allocate memory for new routes");
      return;
    }

    snprintf(new_route, s + 1, route, "%s", route);
    new_route[s] = '\0';
    routes[num_routes] = new_route;

    void **new_c = (void *)malloc(sizeof(void *) * (num_routes + 1));
    if (new_c == NULL) {
      fprintf(stderr, "Failed to allocate memory for new routes");
      return;
    }

    memcpy(new_c, callbacks, sizeof(void *) * (num_routes + 1));
    new_c[num_routes] = ptr;
    free(callbacks);
    callbacks = new_c;

    response_type_t *new_t =
        (response_type_t *)malloc(sizeof(response_type_t) * (num_routes + 1));
    if (new_t == NULL) {
      fprintf(stderr, "Failed to allocate memory for new routes");
      return;
    }

    memcpy(new_t, callback_types, sizeof(response_type_t) * (num_routes));
    free(callback_types);
    callback_types = new_t;
    num_routes++;
  }
}

void update_route(char *route, void *ptr, response_type_t t) {
  int ind = get_route_ind(route);

  if (ind != -1) {
    fprintf(stdout, "Update success\n");
    callbacks[ind] = ptr;
    callback_types[ind] = t;
  } else {
    fprintf(stderr, "Unable to find callback");
  }
}

static char *handle_string_requests(int ind, http_command_t t) {
  char *return_val;
  switch (t) {
  case (GET):
    return (char *)callbacks[ind];
  case (HEAD):
    // the head header will be handled by the http_parser
    // and create the http header response for later
    return (char *)callbacks[ind];
  default:
    return HTTP_BAD_REQUEST;
  };
}

char *route_command(char *route, http_command_t command) {
  int i = get_route_ind(route);
  if (i == -1) {
    fprintf(stderr, "Route wasnt found");
  }
  if (callback_types[i] == STRING) {
    return handle_string_requests(i, command);
  } else {
    // Its up to you to define what a good and bad request is
    http_callback c = (http_callback)callbacks[i];
    return c(route, command);
  }
}

void dump_routes(void) {
  for (int i = 0; i < num_routes; i++) {
    fprintf(stdout, "%s\n", routes[i]);
  }
}

void close_routes(void) {
  for (int i = 0; i < num_routes; i++) {
    free(routes[i]);
  }
  free(callbacks);
  free(callback_types);
  free(routes);
}

response_type_t get_route_type(char *route) {
  int exists = get_route_ind(route);
  if (exists != -1) {
    return callback_types[exists];
  } else {
    return 255;
  }
}
