/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __SERV_H__
#define __SERV_H__

#include "core.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>

typedef	struct	s_serv_comm	t_serv_comm;
typedef	struct	s_serv_clients	t_serv_clients;

struct			s_serv_comm
{
  int			sock_serv;
  struct sockaddr_un	serv_addr;
  //  int			port;
  int			queue_size;
  int			state;
  fd_set		rd_fds;
  int			bigger_fd;
  t_serv_clients	*first_client;
  t_tissue_stack	*general;
};

struct			s_serv_clients
{
  int			sock;
  struct sockaddr_un	client_addr;
  t_serv_clients	*next;
};

#define	ON	1
#define	OFF	0
#define FAIL	-1

extern  t_log_plugin log_plugin;

#endif /* !__SERV_H__ */













