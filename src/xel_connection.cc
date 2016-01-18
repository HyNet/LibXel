/*
 *
 */

#include "xel_connection.h"

xel::connection::connection(int fd)
{
  c_fd = fd;
}

xel::connection::~connection()
{
}

