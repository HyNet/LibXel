/*
 *
 */

#ifndef _XEL_H_INCLUDED_
#define _XEL_H_INCLUDED_

#include <memory>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace xel{

  // forward declaration
  class xel;
  typedef std::shared_ptr<xel> xel_sptr;
  typedef std::weak_ptr<xel> xel_wptr;
  typedef std::shared_ptr<struct sockaddr> SA_sptr;
  typedef std::weak_ptr<struct sockaddr> SA_wptr;

  class epoll;
  typedef std::shared_ptr<epoll> ep_sptr;
  typedef std::weak_ptr<epoll> ep_wptr;
  typedef uintptr_t msec_t;
  typedef enum {
                READ_EVENT =(EPOLLIN|EPOLLRDHUP),
                WRITE_EVENT=(EPOLLOUT)
               } EVENT_TYPE;
  typedef enum {
                ADD_EVENT = EPOLL_CTL_ADD,
                MOD_EVENT = EPOLL_CTL_MOD,
                DEL_EVENT = EPOLL_CTL_DEL
               } ACTION_TYPE;

  class event;
  typedef std::shared_ptr<event> ev_sptr;
  typedef std::weak_ptr<event>   ev_wptr;
  typedef enum { READ, WRITE, ACCEPT } E_TYPE;
  typedef enum { ACTIVE, INACTIVE } E_STATUS;
  typedef std::function<void(int)> HANDLER;

  class connection;
  typedef std::shared_ptr<connection> conn_sptr;
  typedef std::weak_ptr<connection> conn_wptr;

  typedef uintptr_t f_size_t;
  typedef std::vector<ev_sptr >      EVENTS;
  typedef std::vector<conn_sptr > CONNECTIONS;

  class rb_tree;
  class rb_tree_node;
  typedef msec_t rb_tree_key_t;
  typedef enum {BLACK, RED} COLOR;

  class timer;

  class xel{
  public:
    xel();
    ~xel();
    bool init();
    bool destory();
    int  add_event(int fd, EVENT_TYPE type);
    int  del_event(int fd, EVENT_TYPE type);
    int  process_event(void);
    void set_accpet_handler(int fd, HANDLER accept_handler);
    void set_read_handler(int fd, HANDLER read_handler);
    void set_write_handler(int fd, HANDLER write_handler);
    ev_wptr get_event_by_fd(int fd, E_TYPE type);
  private:
    ep_sptr ep;
    EVENTS      revents;
    EVENTS      wevents;
    CONNECTIONS connections;
  };
}

#endif // _XEL_H_INCLUDED_
