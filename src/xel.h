/*
 *
 */

#ifndef _XEL_H_INCLUDED_
#define _XEL_H_INCLUDED_

#include <memory>
#include <vector>

namespace xel{

  // forward declaration
  class epoll;
  typedef std::shared_ptr<epoll> ep_sptr;
  typedef std::weak_ptr<epoll> ep_wptr;

  class event;
  typedef std::shared_ptr<event> ev_sptr;
  typedef std::weak_ptr<event>   ev_wptr;
  typedef enum { READ, WRITE, ACCEPT } E_TYPE;
  typedef enum { ACTIVE, INACTIVE } E_STATUS;
  typedef std::function<void(ev_wptr)> HANDLER;

  class connection;
  typedef std::shared_ptr<connection> conn_sptr;
  typedef std::weak_ptr<connection> conn_wptr;

  class xel;
  typedef std::shared_ptr<xel> xel_sptr;
  typedef std::weak_ptr<xel> xel_wptr;
  typedef uintptr_t f_size_t;
  typedef std::vector<ev_sptr >      EVENTS;
  typedef std::vector<conn_sptr > CONNECTIONS;

  class xel{
  public:
    xel();
    ~xel();
    bool init();
    bool destory();
    ev_wptr get_event_by_fd(int fd, E_TYPE type);
  private:
    ep_sptr ep;
    EVENTS      revents;
    EVENTS      wevents;
    CONNECTIONS connections;
  };
}

#endif // _XEL_H_INCLUDED_