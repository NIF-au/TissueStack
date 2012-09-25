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

#endif /* !__SERV_H__ */













