/*
 *
 */

#ifndef _XEL_EPOLL_H_INCLUDED_
#define _XEL_EPOLL_H_INCLUDED_

#include <sys/epoll.h>
#include <memory>
#include "xel.h"

namespace xel{

  // class epoll;
  // typedef std::shared_ptr<epoll> ep_sptr;
  // typedef std::weak_ptr<epoll> ep_wptr;

  class epoll {
  public:
    typedef int fd;
    typedef int ee_size_t;
    typedef struct epoll_event ep_event;
    typedef struct event_list_s {
                                  ep_event *ee;
                                  int       nevents;
                                } event_list_t;
    // typedef enum {
    //               READ_EVENT =(EPOLLIN|EPOLLRDHUP),
    //               WRITE_EVENT=(EPOLLOUT)
    //              } EVENT_TYPE;
    // typedef enum {
    //               ADD_EVENT = EPOLL_CTL_ADD,
    //               MOD_EVENT = EPOLL_CTL_MOD,
    //               DEL_EVENT = EPOLL_CTL_DEL
    //              } ACTION_TYPE;
  public:
    epoll();
    ~epoll();
    bool init(ee_size_t ee_size = 512);
    bool done(void);
    int  process_event(void);
    int  add_event(ev_wptr ev, EVENT_TYPE type);
    int  del_event(ev_wptr ev, EVENT_TYPE type);
  private:
    epoll(const epoll&){}
    epoll& operator=(const epoll& ep){ if(this != &ep){} return *this; }
    ACTION_TYPE get_event_state(ev_wptr ev, EVENT_TYPE type, bool for_add = true);
  private:
    fd ep = -1;
    event_list_t event_list = {nullptr, 0};
  };
}

#endif //_XEL_EPOLL_H_INCLUDED_
