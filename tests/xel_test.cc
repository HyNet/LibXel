/*
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include "../src/xel.h"
#include "../src/xel_event.h"

static void read_handler(xel::xel_wptr xel, int fd);
static void write_handler(xel::xel_wptr xel, int fd);
static void rprint_handler(xel::xel_wptr xel, int fd)
{
  printf("this is read handler\n");
}

static void wprint_handler(xel::xel_wptr xel, int fd)
{
  printf("this is write handler\n");
}

void nonblocking(int sockfd)
{
  fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
}

void accept_handler(xel::xel_wptr xel, int fd)
{
  int listen_fd = fd;
  int client_fd;
  struct sockaddr_in clientaddr;
  socklen_t len;
  memset(&clientaddr, 0, sizeof(struct sockaddr_in));
  nonblocking(listen_fd);

  do{

    client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &len);
    if (client_fd == -1){
      if (errno == EAGAIN || errno == EWOULDBLOCK){
        // no pending connections are present on the queue
        break;
      } else {
        // TODO: deal with errno left
        return;
      }
    }
    printf("accept listen fd: %d, client fd: %d\n", listen_fd, client_fd);

    nonblocking(client_fd);

    auto r_handler = std::bind(read_handler,xel, std::placeholders::_1);
    auto r_print_handler = std::bind(rprint_handler,xel, std::placeholders::_1);
    auto w_print_handler = std::bind(wprint_handler,xel, std::placeholders::_1);

    // xel.lock()->set_read_handler(client_fd, r_print_handler);
    // xel.lock()->add_event(client_fd, xel::EVENT_TYPE::READ_EVENT);
    // xel.lock()->set_write_handler(client_fd, w_print_handler);
    // xel.lock()->add_event(client_fd, xel::EVENT_TYPE::WRITE_EVENT);

    xel.lock()->set_read_handler(client_fd, r_handler);

    xel.lock()->add_event(client_fd, xel::EVENT_TYPE::READ_EVENT);
    // notice: do not add write event here!
    // use send function directly in the read handler
    // or add write event when the send function return with errno == EAGAIN;
  } while(client_fd > 0);

}

void read_handler(xel::xel_wptr xel, int fd)
{
  const int buf_size = 1024;
  char buf[buf_size] ={0};
  ssize_t cnt = 0;
  ssize_t nread = 0;

  do {
     nread = recv(fd, buf + cnt, buf_size, 0);
     printf("recv: %ld byte data\n", nread);
     if(nread == -1){
       if (errno == EAGAIN || errno == EWOULDBLOCK) {
         // nothing left to read;
         break;
       } else {
         // error happend
         return;
       }
     }
     if(nread == 0){
       // on condition:
       // 1. nothing left to read
       // 2. client connect to server, close connect without calling send
     }
     if(nread > 0){
       cnt += nread;
     }
  } while(nread > 0 && cnt <= buf_size);
  printf("%s\n", buf);
  write_handler(xel, fd);
  close(fd);
}

void write_handler(xel::xel_wptr xel, int fd)
{
  char buf[1024];
  sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: 37\r\n\r\n%s",
          "<html><body>hello world</body></html>");
  int len = strlen(buf);
  ssize_t nleft = len;
  ssize_t n = 0;

  do {
    n = send(fd, buf + len - nleft, nleft, 0);
    printf("send: %ld bytes data", n);
    if (n == -1){
      if (errno == EAGAIN || errno == EWOULDBLOCK){
        // no blocking mode failed send
        // TODO: in this case just add write event to epoll
        break;
      } else {
        // error happend;
        return;
      }
    }
    if(n > 0){
      nleft -= n;
    }
  } while(n > 0 && nleft > 0);
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

  const int port = 8000;
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(port);

  if (bind(sock_fd, (struct sockaddr *)&serveraddr,
           sizeof(struct sockaddr_in)) == -1){
    perror(strerror(errno));
    exit(1);
  }

  if (listen(sock_fd, 1024) == -1){
    perror(strerror(errno));
    exit(1);
  }

  printf("listen on fd: %d, port: %d\n", sock_fd, port);

  // this is interesting here:
  // we get a partial function object, it's a clever idea :)
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
