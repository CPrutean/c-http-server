#include <global_includes.h>
#include <http_parser.h>
#include <router.h>
#include <server.h>
#include <thread_pool.h>
#include <unistd.h>

int main() {

  printf("Starting http server on port 8080\n");
  init_routes();
  update_route("/", "Hello world", STRING);
  int sockfd = create_server_socket(8080);
  start_server(sockfd);
  while (1) {
    sleep(1);
  }

  clean_workers();
  close_routes();

  return 0;
}
