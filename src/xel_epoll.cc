/*
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "xel.h"
#include "xel_epoll.h"
#include "xel_event.h"
#include "xel_connection.h"

xel::epoll::epoll()
{
}

xel::epoll::~epoll()
{
}

bool xel::epoll::init(ee_size_t ee_size /*512*/)
{
  ep = epoll_create(512);
  if (ep == -1){
    perror(strerror(errno));
    return false;
  }
  event_list.ee = (ep_event*)malloc(sizeof(ep_event) * ee_size);
  if (event_list.ee == NULL){
    perror(strerror(errno));
    return false;
  }
  event_list.nevents = ee_size;
  return true;
}

bool xel::epoll::done(void)
{
  if (close(ep) == -1){
    perror(strerror(errno));
    return false;
  }
  free(event_list.ee);
  event_list.ee = nullptr;
  event_list.nevents = 0;
  return true;
}


int xel::epoll::process_event(msec_t timer /*-1*/)
{
  int err;
  uint32_t revent;
  connection *c = NULL;
  ev_wptr rev, wev;

  int nevent = epoll_wait(ep, event_list.ee, event_list.nevents, timer);
  err = (nevent == -1)? errno: 0;
  if (err){
    // TODO: err == EINTR when timeout
    perror(strerror(errno));
    return -1;
  }

  if (nevent == 0){
    // TODO: err == EINTR when timeout
    return -1;
  }

  for(int i = 0; i < nevent; i++){
    // TODO: find a better way to deal with the raw pointer
    c = static_cast<connection *>(event_list.ee[i].data.ptr);
    revent = event_list.ee[i].events;
    rev = c->read_event();
    wev = c->write_event();
    if (revent & (EPOLLERR|EPOLLHUP)){
      // epoll_wait error on fd: %d, event:
    }

    if (revent & EPOLLIN){
      if(rev.lock()->handler()){
        rev.lock()->handler()(rev.lock()->fd());
      }
    }
    if (revent & EPOLLOUT){
      if(rev.lock()->handler()){
        wev.lock()->handler()(wev.lock()->fd());
      }
    }
  }
  return 0;
}

int xel::epoll::del_event(ev_wptr ev, EVENT_TYPE type, int flag /*EPOLLET*/)
{
  ep_event ee;
  ACTION_TYPE op;
  EVENT_TYPE prev;
  ev_wptr e;
  conn_wptr c;


  if (ev.expired()){
    // TODO: error log
    return -1;
  }

  c = ev.lock()->conn();

  if(type == EVENT_TYPE::READ_EVENT){
    e = c.lock()->write_event();
    prev = EVENT_TYPE::WRITE_EVENT;
  } else {
    e = c.lock()->read_event();
    prev = EVENT_TYPE::READ_EVENT;
  }

  if(e.lock()->status() == E_STATUS::ACTIVE){
    op = ACTION_TYPE::MOD_EVENT;
    ee.events = prev | flag;
    ee.data.ptr = (void *)(ev.lock()->conn().lock().get());
  } else {
    op = ACTION_TYPE::DEL_EVENT;
    ee.events = 0;
    ee.data.ptr = nullptr;
    ev.lock()->set_status(E_STATUS::INACTIVE);
    return 0;
  }

  // ACTION_TYPE op = get_event_state(ev, type, false);
  // switch(op){
  // case ACTION_TYPE::DEL_EVENT:
  //   ee.events = 0;

  if (epoll_ctl(ep, op, ee.data.fd, &ee) == -1){
    perror(strerror(errno));
    return -1;
  }
  ev.lock()->set_status(E_STATUS::INACTIVE);
  return 0;
}

int xel::epoll::add_event(ev_wptr ev, EVENT_TYPE type, int flag /*EPOLLET*/)
{
  ep_event ee;
  uint32_t events, prev;
  ACTION_TYPE op;
  ev_wptr e;
  conn_wptr c;

  if(ev.expired()){
    // TODO: error log
    return -1;
  }
  c = ev.lock()->conn();
  events = type;
  if (type == EVENT_TYPE::READ_EVENT){
    e = c.lock()->write_event();
    prev = EVENT_TYPE::WRITE_EVENT;
  } else {
    e = c.lock()->read_event();
    prev = EVENT_TYPE::READ_EVENT;
  }

  if(e.lock()->status() == E_STATUS::ACTIVE){
    op = ACTION_TYPE::MOD_EVENT;
    events |= prev;
  } else {
    op = ACTION_TYPE::ADD_EVENT;
  }

  ee.events = events | flag;
  ee.data.ptr = (void *)(ev.lock()->conn().lock().get());

  if (epoll_ctl(ep, op, ev.lock()->fd(), &ee) == -1){
    perror(strerror(errno));
    return -1;
  }
  ev.lock()->set_status(E_STATUS::ACTIVE);
  return 0;
}

