#include "server.h"
#include "thread_pool.h"
#include <unistd.h>

static int max_connections = 10;
static volatile int running = 0;

void start_server(int sockfd) { init_threads(max_connections, sockfd); }
void stop_server(void) { running = 0; }
void set_max_connections(int connections) { max_connections = connections; }
int get_max_connections(void) { return max_connections; }
int check_connections(int sockfd) { return listen(sockfd, max_connections); }

void generate_hints(struct addrinfo *hints) {
  memset(hints, 0, sizeof(struct addrinfo));
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_flags = AI_PASSIVE;
}

int create_server_socket(int port) {
  int status;
  struct addrinfo hints;
  struct addrinfo *res;
  generate_hints(&hints);
  char port_str[6];
  snprintf(port_str, 6, "%d", port);

  status = getaddrinfo(NULL, port_str, &hints, &res);
  if (status != 0) {
    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
    exit(1);
  }
  // get the socket descriptor

  int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fd == -1) {
    fprintf(stderr, "socket error");
    exit(1);
  }

  // Bind the process to this socket
  int b = bind(fd, res->ai_addr, res->ai_addrlen);
  if (b != 0) {
    fprintf(stderr, "bind error");
    exit(1);
  }

  if (listen(fd, 10) == -1) {
    fprintf(stderr, "Listen failed\n");
    close(fd);
  }

  freeaddrinfo(res);
  return fd;
}
