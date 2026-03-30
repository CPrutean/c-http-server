#include "thread_pool.h"

static volatile int run_workers = 1;
static int num_threads;
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
  while (run_workers) {
    pthread_mutex_lock(&queue_mutex);

    while (is_task_empty()) {
      pthread_cond_wait(&thread_cond, &queue_mutex);
    }

    int sfd = get_sock_fd();
    // Implement actual work to be done

    pthread_mutex_unlock(&queue_mutex);
  }
  return NULL;
}

void init_threads(int n_threads) {
  num_threads = n_threads;
  thread_list = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
  thread_valid = (int *)calloc(num_threads, sizeof(int));
  list.list = (int *)malloc(sizeof(int) * n_threads * 2);
  list.capacity = n_threads * 2;
  for (int i = 0; i < n_threads; i++) {
    if (pthread_create(&thread_list[i], NULL, worker_thread, NULL) != 0) {
      printf("Thread number %d failed to init\n", i);
      thread_valid[i] = 0;
    } else {
      thread_valid[i] = 1;
    }
  }
}

void stop_worker_threads(void) { run_workers = 0; }

void clean_workers(void) {
  for (int i = 0; i < num_threads; i++) {
    if (thread_valid[i]) {
      pthread_join(thread_list[i], NULL);
    }
  }
  free(thread_valid);
  free(thread_list);
  free(list.list);
}

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
