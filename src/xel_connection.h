/*
 *
 */

#ifndef _XEL_CONNECTION_H_INCLUDED_
#define _XEL_CONNECTION_H_INCLUDED_

#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include "xel.h"

namespace xel {
  // class connection;
  // typedef std::shared_ptr<connection> conn_sptr;
  // typedef std::weak_ptr<connection> conn_wptr;

  class connection {
  public:
    connection(int fd);
    ~connection();

    inline void set_read_event(ev_wptr rev){ revent = rev; }
    inline void set_write_event(ev_wptr wev){ wevent = wev; }
    inline ev_wptr read_event(){ return revent; }
    inline ev_wptr write_event(){ return wevent; }
    inline int fd(){ return c_fd; }
  private:
    int c_fd;
    ev_wptr revent;
    ev_wptr wevent;
    struct sockaddr *sockaddr;
  };
}

#endif // _XEL_CONNECTION_H_INCLUDED_
