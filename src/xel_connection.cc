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

void xel::connection::set_sockaddr(short family, unsigned long addr, short port)
{
}

