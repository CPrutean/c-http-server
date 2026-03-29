#include "thread_pool.h"

static int num_threads;
static pthread_t *thread_list;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;

struct sockfd_list {
  int *list;
  int size;
  int capacity;
};

static struct sockfd_list list;
static int get_sock_fd();
static void add_sock_fd();
static int is_task_empty();

static void *worker_thread(void *args) {
  while (1) {
    pthread_mutex_lock(&queue_mutex);

    while (list.size == 0) {
      pthread_cond_wait(&thread_cond, &queue_mutex);
    }

    pthread_mutex_unlock(&queue_mutex);
  }
}
void init_threads(int n_threads) {
  num_threads = n_threads;
  thread_list = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
  list.list = (int *)malloc(sizeof(int) * n_threads * 2);
  list.capacity = n_threads * 2;
  for (int i = 0; i < n_threads; i++) {
    if (pthread_create(&thread_list[i], NULL, worker_thread, NULL) != 0) {
      printf("Thread number %d failed to init\n", i);
    }
  }
}

int get_sock_fd() {

  pthread_mutex_lock(&queue_mutex);
  if (list.size <= 0) {
    pthread_mutex_unlock(&queue_mutex);
    return NULL;
  }
  int sfd = list.list[list.size - 1];
  list.size--;
  pthread_mutex_unlock(&queue_mutex);
  return sfd;
}

void add_sock_fd(int sockfd) {
  pthread_mutex_lock(&queue_mutex);
  if (list.size >= list.capacity) {
    int *new_buff = (int *)malloc(sizeof(int) * list.capacity * 2);
    memcpy(new_buff, list.list, list.capacity);
    free(list.list);
    list.capacity *= 2;
    list.list = new_buff;
  }

  list.list[list.size++] = sockfd;
  pthread_mutex_unlock(&queue_mutex);
}

int is_task_empty() {
  pthread_mutex_lock(&queue_mutex);
  int cond = list.size != 0;
  pthread_mutex_unlock(&queue_mutex);
  return cond;
}
