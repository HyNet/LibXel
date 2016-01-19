/*
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../src/xel.h"
#include "../src/xel_event.h"

static void read_handler(xel::xel_wptr xel, int fd);
static void write_handler(xel::xel_wptr xel, int fd);

void accept_handler(xel::xel_wptr xel, int fd)
{
  int listen_fd = fd;
  int client_fd;
  struct sockaddr_in clientaddr;
  socklen_t len;
  memset(&clientaddr, 0, sizeof(struct sockaddr_in));
  client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &len);
  if(client_fd == -1){
    perror(strerror(errno));
    exit(1);
  }
  printf("accept listen fd: %d, client fd: %d\n", listen_fd, client_fd);

  auto r_handler = std::bind(read_handler,xel, std::placeholders::_1);
  auto w_handler = std::bind(write_handler,xel, std::placeholders::_1);
  xel.lock()->set_read_handler(client_fd, r_handler);
  xel.lock()->set_write_handler(client_fd, w_handler);

  xel.lock()->add_event(client_fd, xel::EVENT_TYPE::READ_EVENT);
  xel.lock()->add_event(client_fd, xel::EVENT_TYPE::WRITE_EVENT);

}

void read_handler(xel::xel_wptr xel, int fd)
{
  printf("this is read handler\n");
}

void write_handler(xel::xel_wptr xel, int fd)
{
  printf("this is read handler\n");
}

int main(int argc, char **argv)
{
  xel::xel_sptr el = std::make_shared<xel::xel>();
  el->init();

  struct sockaddr_in serveraddr;
  memset(&serveraddr, 0, sizeof(struct sockaddr_in));

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1){
    perror(strerror(errno));
    exit(1);
  }

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(8000);

  if (bind(sock_fd, (struct sockaddr *)&serveraddr,
           sizeof(struct sockaddr_in)) == -1){
    perror(strerror(errno));
    exit(1);
  }

  if (listen(sock_fd, 1024) == -1){
    perror(strerror(errno));
    exit(1);
  }

  printf("listen on fd: %d, port: %d\n", sock_fd, 8000);

  // this is intersting here:
  // we get a partial function object, it's a clear idea :)
  auto handler = std::bind(accept_handler,
                           xel::xel_wptr(el),
                           std::placeholders::_1);

  el->set_accpet_handler(sock_fd, handler);
  el->add_event(sock_fd, xel::EVENT_TYPE::READ_EVENT);
  for(;;){
    el->process_event();
  }

  el->destory();
  return 0;
}
