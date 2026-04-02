#include "thread_pool.h"
#include "global_includes.h"
#include "http_parser.h"
#include "router.h"
#include <unistd.h>

#define RECV_BUFF_S 1024

static volatile int run_workers = 1;
static int num_threads;
static pthread_t discovery_thread;
static pthread_t *thread_list;
static int *thread_valid;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;

struct sockfd_list {
  int *list;
  struct addrinfo *addrlist;
  int size;
  int capacity;
};

static struct sockfd_list list;

// all of these functions assume that you have called with THE LOCK ACQUIRED
// NEVER USE IF YOU ARENT LOCKING BEFORE HAND
static int get_sock_fd(void);
static void add_sock_fd(int sockfd);
static int is_task_empty(void);

static void *worker_thread(void *args) {
  char buffer[RECV_BUFF_S];
  memset(buffer, 0, sizeof(buffer));

  while (run_workers) {
    pthread_mutex_lock(&queue_mutex);
    while (is_task_empty()) {
      pthread_cond_wait(&thread_cond, &queue_mutex);
    }

    int sfd = get_sock_fd();

    pthread_mutex_unlock(&queue_mutex);

    memset(buffer, 0, sizeof(buffer));
    // Implement actual work to be done
    size_t s = recv(sfd, buffer, sizeof(buffer) - 1, 0);
    fprintf(stdout, "Message received: %s\n", buffer);
    if (s >= 0) {

      struct http_info *inf = parse_http_request(buffer);
      fprintf(stderr, "Parse success\n");
      if (inf == NULL) {
        fprintf(stderr, "Failed to parse request\n");
        continue;
      }

      printf(
          "Info dump\n Route: %s\n MetaData: %s\n Conent: %s\n, HttpVsn: %s\n",
          inf->route, inf->metadata, inf->content, inf->http_version);

      response_type_t t = get_route_type(inf->route);
      fprintf(stderr, "Found response success\n");
      if (t == 255) {
        fprintf(stderr, "INVALID ROUTE");
        send(sfd, HTTP_BAD_REQUEST, strlen(HTTP_BAD_REQUEST), 0);

        continue;
      }

      char *response = route_command(inf->route, inf->request_t);
      char *response_full = post_response(inf, response, t);

      fprintf(stderr, "Message success\n");
      size_t s_send = send(sfd, response_full, strlen(response_full), 0);

      if (s_send != strlen(response_full)) {
        while (s_send < strlen(response_full)) {
          s_send += send(sfd, (response_full + s_send),
                         strlen(response_full) - s_send, 0);
        }
      }

      free(response_full);
      free_http_info(inf);
      if (t == FUNC) {
        free(response);
      }
    } else {
      fprintf(stderr, "message parse fail\n");
    }
    close(sfd);
  }
  return NULL;
}

static void *discovery_thread_task(void *args) {
  int sfd;
  memcpy(&sfd, args, sizeof(int));
  while (run_workers) {

    struct sockaddr_in adr;
    socklen_t adr_size = sizeof(adr);
    int new_fd = accept(sfd, (struct sockaddr *)&adr, &adr_size);
    if (new_fd != -1) {
      dispatch_connection(new_fd);
      fprintf(stderr, "Accept success\n");
    } else {
      fprintf(stderr, "Accept failed\n");
    }
  }
  return NULL;
}

void init_threads(int n_threads, int sockfd) {
  num_threads = n_threads;
  thread_list = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
  thread_valid = (int *)calloc(num_threads, sizeof(int));
  list.list = (int *)malloc(sizeof(int) * n_threads * 2);
  list.capacity = n_threads * 2;
  for (int i = 0; i < n_threads; i++) {
    if (pthread_create(&thread_list[i], NULL, worker_thread, NULL) != 0) {
      fprintf(stderr, "Thread number %d failed to init\n", i);
      thread_valid[i] = 0;
    } else {
      thread_valid[i] = 1;
    }
  }

  int *sfdptr = (int *)malloc(sizeof(int));
  *sfdptr = sockfd;
  if (pthread_create(&discovery_thread, NULL, discovery_thread_task, sfdptr) !=
      0) {
    fprintf(stderr, "Failed discovery thread");
  }
}

void clean_workers(void) {
  pthread_mutex_lock(&queue_mutex);
  run_workers = 0;
  pthread_cond_broadcast(&thread_cond);
  pthread_mutex_unlock(&queue_mutex);

  for (int i = 0; i < num_threads; i++) {
    if (thread_valid[i]) {
      pthread_join(thread_list[i], NULL);
    }
  }
  free(thread_valid);
  free(thread_list);
  free(list.list);
}
void stop_worker_threads(void) { run_workers = 0; }

static int get_sock_fd(void) {
  if (list.size <= 0) {
    return -1;
  }
  int sfd = list.list[list.size - 1];
  list.size--;
  return sfd;
}

static void add_sock_fd(int sockfd) {
  if (list.size >= list.capacity) {
    int *new_buff = (int *)malloc(sizeof(int) * list.capacity * 2);
    memcpy(new_buff, list.list, list.capacity * sizeof(int));
    free(list.list);
    list.capacity *= 2;
    list.list = new_buff;
  }

  list.list[list.size++] = sockfd;
}

static int is_task_empty(void) {
  int cond = list.size == 0;
  return cond;
}

void dispatch_connection(int sockfd) {
  pthread_mutex_lock(&queue_mutex);
  add_sock_fd(sockfd);
  pthread_cond_signal(&thread_cond);
  pthread_mutex_unlock(&queue_mutex);
}
