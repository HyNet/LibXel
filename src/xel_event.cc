/*
 *
 */

#include "xel_event.h"

xel::event::event(int fd, E_TYPE type)
{
  e_fd = fd;
  e_type = type;
  e_status = INACTIVE;
}

xel::event::~event()
{
}
