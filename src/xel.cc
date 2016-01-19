/*
 *
 */

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include "xel.h"
#include "xel_epoll.h"
#include "xel_event.h"
#include "xel_connection.h"

xel::xel::xel()
{
}

xel::xel::~xel()
{
}

bool xel::xel::init()
{
  struct rlimit rlmt;
  f_size_t nfiles;
  if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1){
    perror(strerror(errno));
    return false;
  }
  nfiles = static_cast<uintptr_t>(rlmt.rlim_cur);
  ep = std::make_shared<epoll>();
  ep->init();
  for(f_size_t i = 0; i < nfiles; i++){
    revents.push_back(std::make_shared<event>(i, E_TYPE::READ));
    wevents.push_back(std::make_shared<event>(i, E_TYPE::WRITE));
    connections.push_back(std::make_shared<connection>(i));
    revents[i]->set_conn(connections[i]);
    wevents[i]->set_conn(connections[i]);
    connections[i]->set_read_event(revents[i]);
    connections[i]->set_write_event(wevents[i]);
  }
  return true;
}

bool xel::xel::destory()
{
  return ep->done();
}

xel::ev_wptr xel::xel::get_event_by_fd(int fd, E_TYPE type)
{
  switch(type){
  case E_TYPE::READ:
    return revents[fd];
  case E_TYPE::ACCEPT:
    return revents[fd];
  case E_TYPE::WRITE:
    return wevents[fd];
  default:
    // TODO: error log
    break;
  }
  return ev_wptr();
}

int xel::xel::add_event(int fd, EVENT_TYPE type)
{
  ev_wptr ev = (type == EVENT_TYPE::READ_EVENT ?
                        revents[fd]: wevents[fd]);
  return ep->add_event(ev, type);
}

int xel::xel::del_event(int fd, EVENT_TYPE type)
{
  ev_wptr ev = (type == EVENT_TYPE::READ_EVENT ?
                        revents[fd]: wevents[fd]);
  return ep->del_event(ev, type);
}

int xel::xel::process_event()
{
  return ep->process_event();
}
