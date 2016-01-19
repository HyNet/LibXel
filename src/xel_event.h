/*
 *
 */

#ifndef _XEL_EVENT_H_INCLUDED_
#define _XEL_EVENT_H_INCLUDED_

#include <memory>
#include <functional>
#include "xel.h"

namespace xel {
  // class event;
  // typedef std::shared_ptr<event> ev_sptr;
  // typedef std::weak_ptr<event>   ev_wptr;

  class event {
  public:
    // typedef enum {
    //               READ,
    //               WRITE,
    //               ACCEPT
    //              } E_TYPE;
    // typedef enum { ACTIVE, INACTIVE } E_STATUS;
    // typedef std::function<void(ev_wptr ev)> HANDLER;
  public:
    event(int fd, E_TYPE type);
    ~event();
    inline void set_handler(HANDLER handler){ e_handler = handler; }
    inline void set_conn(conn_wptr c){ e_conn = c; }
    inline void set_event_type(E_TYPE type) { e_type = type; }
    inline void set_status(E_STATUS status) { e_status = status; }
    inline int fd() { return e_fd; }
    inline E_TYPE type() { return e_type; }
    inline conn_wptr conn(){ return e_conn; }
    inline HANDLER handler(){ return e_handler; }
    inline E_STATUS status(){ return e_status; }
  private:
    event(const event&){}
    const event& operator=(const event& e){ if(this != &e){} return *this; }
  private:
    int e_fd;
    E_TYPE e_type;
    E_STATUS e_status = E_STATUS::INACTIVE;
    HANDLER e_handler;
    conn_wptr e_conn;
  };
}

#endif // _XEL_EVENT_H_INCLUDED_
