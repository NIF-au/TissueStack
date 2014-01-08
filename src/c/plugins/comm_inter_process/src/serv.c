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
#include "serv.h"

void		add_client_to_list(t_serv_comm *s, int socket,
				   struct sockaddr_un client_addr)
{
  t_serv_clients *c;

  if ((c = s->first_client) == NULL)
    {
      s->first_client = malloc(sizeof(*s->first_client));
      c = s->first_client;
    }
  else
    {
      while (c->next != NULL)
	c = c->next;
      c->next = malloc(sizeof(*c->next));
      c = c->next;
    }
  c->next = NULL;
  c->sock = socket;
  c->client_addr = client_addr;

  // peut etre mettre la socket en O_NONBLOCK
}

void		add_client_to_sock_monitor(t_serv_comm *s, int socket)
{
  if (s->bigger_fd < socket)
    s->bigger_fd = socket;
  FD_SET(socket, &(s->rd_fds));
}

void		serv_accept_new_connections(t_serv_comm *s)
{
  int		socket;
  struct sockaddr_un	client_addr;
  socklen_t	len;

  len = sizeof(client_addr);
  if ((socket = accept(s->sock_serv, (struct sockaddr*)&client_addr,
		       &len)) == -1)
    {
      ERROR("Accept Error");
      s->state = FAIL;
      return;
    }

  DEBUG("SERVER CONNECT %i\n", socket);

  add_client_to_list(s, socket, client_addr);
  add_client_to_sock_monitor(s, socket);
}

void		client_diconnected(t_serv_comm *s, int sock)
{
  t_serv_clients *c;
  t_serv_clients *tmp;

  // This is necessary! We have to close the unix socket server and client side !!!
  shutdown(sock, 2);
  close(sock);

  DEBUG("SERVER DISCONNECT %i\n", sock);

  c = s->first_client;

  if (c)
    {
      if (c->sock == sock)
	{
	  free(s->first_client);

	  s->first_client = NULL;
	  c = NULL;
	}
  }
  c = s->first_client;
  while (c != NULL)
    {
      if (c->sock == sock)
	{
	  tmp->next = c->next;
	  free(c);
	  break;
	}
      tmp = c;
      c = c->next;
    }
}

void		check_modified_fd(t_serv_comm *s)
{
  t_serv_clients *c;
  char		buff[4096];
  int		len;

  if (FD_ISSET(s->sock_serv, &(s->rd_fds)))
      serv_accept_new_connections(s);
  else
    {
      c = s->first_client;
      while (c != NULL)
	{
	  if (FD_ISSET(c->sock, &(s->rd_fds)))
	    {
	      // Do stuff;
	      len = read(c->sock, buff, 4096);
	      if (len <= 0)
		{
		  client_diconnected(s, c->sock);
		  break;
		}
	      buff[len] = '\0';
	      (*s->general->plug_actions)(s->general, buff, &c->sock);
	      break;
	    }
	  c = c->next;
	}
    }
}

void		reset_set_fd_to_monitor(t_serv_comm *s)
{
	if (s == NULL) return;

  t_serv_clients *c = NULL;

  s->bigger_fd = 0;
  FD_ZERO(&(s->rd_fds));
  FD_SET(s->sock_serv, &(s->rd_fds));
  if (s->sock_serv > s->bigger_fd)
    s->bigger_fd = s->sock_serv;
  c = s->first_client;
  while (c != NULL)
    {
      FD_SET(c->sock, &(s->rd_fds));
      if (c->sock > s->bigger_fd)
	s->bigger_fd = c->sock;
      c = c->next;
    }
}

void		serv_working_loop(t_serv_comm *s)
{
  s->state = ON;
  //  FD_ZERO(&(s->rd_fds));
  add_client_to_sock_monitor(s, s->sock_serv);
  while (s->state != OFF && s->state != FAIL)
    {
      reset_set_fd_to_monitor(s);
      if (select((s->bigger_fd + 1), &(s->rd_fds),
		 NULL, NULL, NULL) == -1)
	{
      perror("select");
	  ERROR("Select Error");
	  //>state = FAIL;
	}
      check_modified_fd(s);
    }
}

int		serv_init_connect(t_serv_comm *s)
{
  if ((s->sock_serv = socket(AF_UNIX/*AF_INET*/, SOCK_STREAM, 0)) < 0)
    {
      ERROR("Socket creation Error");
      return (-1);
    }
  s->queue_size = 50;
  unlink("/tmp/tissue_stack_communication");
  memset(&(s->serv_addr), 0, sizeof(s->serv_addr));
  s->serv_addr.sun_family = AF_UNIX; //AF_INET;
  snprintf(s->serv_addr.sun_path, 108, UNIX_SOCKET_PATH);
  //  s->serv_addr.sin_addr.s_addr = INADDR_ANY;
  //  s->serv_addr.sin_port = htons(s->port);
  if (bind(s->sock_serv, (struct sockaddr*)&(s->serv_addr),
	   sizeof(s->serv_addr)) < 0)
    {
      ERROR("Bind Error");
      return (-1);
    }
  chmod(UNIX_SOCKET_PATH, 0666);
  listen(s->sock_serv, s->queue_size);
  return (0);
}

t_serv_comm	*s;

void		*init(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  LOG_INIT(a);
  s = NULL;
  INFO("Inter Process Communicator initialized.");
  return (NULL);
}

void		*start(void *args)
{
  prctl(PR_SET_NAME, "TS_INT_COMM");
  t_args_plug	*a = (t_args_plug*)args;
  s = malloc(sizeof(*s));
  s->bigger_fd = 0;
  s->general = a->general_info;
  a->this->stock = s;
  serv_init_connect(s);
  serv_working_loop(s);
  return (NULL);
}

void		*unload(void *args)
{
	if (s != NULL) {
		free(s);
		s = NULL;
	}
	INFO("Inter Process Communicator Plugin: Unloaded");

	return (NULL);
}
