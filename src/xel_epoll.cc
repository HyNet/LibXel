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


int xel::epoll::process_event(void)
{
  int err;
  uint32_t revent;
  connection *c;
  ev_wptr rev, wev;

  int nevent = epoll_wait(ep, event_list.ee, event_list.nevents, -1);
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
        rev.lock()->handler()(rev);
      }
    }
    if (revent & EPOLLOUT){
      if(rev.lock()->handler()){
        wev.lock()->handler()(rev);
      }
    }
  }
  return 0;
}

int xel::epoll::del_event(ev_wptr ev, EVENT_TYPE type)
{
  ep_event ee;

  if (ev.expired()){
    // TODO: error log
    return -1;
  }

  ACTION_TYPE op = get_event_state(ev, type, false);
  switch(op){
  case ACTION_TYPE::DEL_EVENT:
    ee.events = 0;
    ee.data.ptr = nullptr;
    break;
  case ACTION_TYPE::MOD_EVENT:
    ee.events = (type == EVENT_TYPE::READ_EVENT?
                         EVENT_TYPE::WRITE_EVENT:
                         EVENT_TYPE::READ_EVENT);
    // TODO: find a better way to deal with
    //       the transfer from shared_ptr to raw pointer
    //       for now it is safe,
    //       because of the global lifecycle of events class
    ee.data.ptr = (void *)(ev.lock()->conn().lock().get());
    break;
  default:
    // TODO: error log
    break;
  }

  if (epoll_ctl(ep, op, ee.data.fd, &ee) == -1){
    perror(strerror(errno));
    return -1;
  }
  return 0;
}

int xel::epoll::add_event(ev_wptr ev, EVENT_TYPE type)
{
  ep_event ee;
  uint32_t events;

  if(ev.expired()){
    // TODO: error log
    return -1;
  }

  ACTION_TYPE op = get_event_state(ev, type);
  switch(op){
  case ACTION_TYPE::ADD_EVENT:
    events = (type == EVENT_TYPE::READ_EVENT?
                  EVENT_TYPE::READ_EVENT :
                  EVENT_TYPE::WRITE_EVENT);
    break;
  case ACTION_TYPE::MOD_EVENT:
    events = EVENT_TYPE::READ_EVENT|EVENT_TYPE::WRITE_EVENT;
    break;
  default:
    // TODO: error log; happened when DEL_EVENT
    //       I think this will never happend :)
    events = EVENT_TYPE::READ_EVENT|EVENT_TYPE::WRITE_EVENT;
    break;
  }

  ee.events = events;
  ee.data.ptr = (void *)(ev.lock()->conn().lock().get());

  if (epoll_ctl(ep, op, ev.lock()->fd(), &ee) == -1){
    perror(strerror(errno));
    return -1;
  }
  return 0;
}

xel::ACTION_TYPE
xel::epoll::get_event_state(ev_wptr ev, EVENT_TYPE type, bool for_add /*true*/)
{
  conn_wptr c = ev.lock()->conn();
  ACTION_TYPE action = for_add? ACTION_TYPE::ADD_EVENT: ACTION_TYPE::DEL_EVENT;
  E_STATUS prev = E_STATUS::INACTIVE;

  switch(type){
  case READ_EVENT:
    prev = c.lock()->write_event().lock()->status();
    action = (prev == E_STATUS::ACTIVE?
                          ACTION_TYPE::MOD_EVENT:
                          action);
    break;
  case WRITE_EVENT:
    prev = c.lock()->read_event().lock()->status();
    action = (prev == E_STATUS::ACTIVE?
                          ACTION_TYPE::MOD_EVENT:
                          action);
    break;
  default:
    // TODO: error log, enum type keep this away :)
    break;
  }
  return action;
}
