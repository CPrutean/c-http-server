#include "router.h"

static char **routes;
static void **callbacks;
static response_type_t *callback_types;
static int num_routes;

static int get_route_ind(char *route) {
  int found = 0;
  int i;
  for (i = 0; i < num_routes; i++) {
    if (strncmp(routes[i], route, strlen(routes[i])) == 0) {
      found = 1;
      break;
    }
  }
  if (found) {
    return i;
  } else {
    return -1;
  }
}

void init_routes(void) {
  routes = (char **)malloc(sizeof(char *) * 2);
  for (int i = 0; i < 2; i++) {
    routes[i] = (char *)malloc(sizeof(char) * 6);
  }
  callbacks = (void *)malloc(sizeof(void *) * 2);
  snprintf(routes[0], 6, "%s", "/");
  routes[0][1] = '\0';
  callbacks[0] = NULL;

  snprintf(routes[1], 6, "%s", "/root");
  routes[1][6] = '\0';
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
    memcpy(new_r, routes, sizeof(char *) * (num_routes + 1));
    free(routes);
    routes = new_r;

    char *new_route = (char *)malloc(sizeof(char) * (s + 1));
    if (new_route == NULL) {
      fprintf(stderr, "Failed to allocate memory for new routes");
      return;
    }

    snprintf(new_route, s + 1, route, "%s");
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

  if (ind == -1) {
    callbacks[ind] = ptr;
    callback_types[ind] = t;
  } else {
    fprintf(stderr, "Unable to find callback");
  }
}

char *return_route(char *route) {
  int ind = get_route_ind(route);
  if (ind == -1) {
    return NULL;
  }

  if (callback_types[ind] == STRING) {
    return (char *)callbacks;
  } else {
    http_callback c = (http_callback)callbacks[ind];
    char *r = c(route);
    return r;
  }
}

void dump_routes(void) {}

void close_routes(void) {}
