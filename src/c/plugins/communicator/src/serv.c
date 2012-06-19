#include "serv.h"

char		*str_n_cpy(char *str, int position, int len)
{
  char		*dest;
  int		i;

  i = 0;
  dest = malloc((len + 1) * sizeof(*dest));
  while (i < len)
    {
      dest[i] = str[i + position];
      i++;
    }
  dest[i] = '\0';
  return (dest);
}

int		serv_word_count(char *buff, char c)
{
  int		i;
  int		count;

  i = 0;
  count = 0;
  if (buff[0] != c && buff[0] != '\0')
    count++;
  while (buff[i] != '\0')
    {
      if ((buff[i] == c && buff[i + 1] != c ) && buff[i + 1] != '\0')
	count++;
      i++;
    }
  return (count);
}

int		serv_letter_count(char *buff, int position, char c)
{
  int		i;

  i = 0;
  while (buff[position + i] != c && buff[position + i] != '\0')
    i++;
  return (i);
}

char		**serv_str_to_wordtab(char *buff, char c)
{
  int		i;
  int		j;
  char		**dest;

  dest = malloc((serv_word_count(buff, c) + 1) * sizeof(*dest));
  i = 0;
  j = 0;
  while (buff[i] != '\0')
    {
      if (buff[i] != c && buff[i] != '\0')
	{
	  dest[j] = str_n_cpy(buff, i, serv_letter_count(buff, i, c));
	  j++;
	  i += serv_letter_count(buff, i, c);
	}
      if (buff[i] != '\0')
	i++;
    }
  dest[j] = NULL;
  return (dest);
}

void		write_header(int socket)
{
  char		header[4096];
  int		len;

  len = sprintf(header,
		"HTTP/1.1 200 OK\r\n"
		"Dat: Thu, 20 May 2004 21:12:11 GMT\r\n"
		"Connection: close\r\n"
		"Server: TissueStack Server\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Type: image/png\r\n"
		"Last-Modified: Thu, 20 May 2004 21:12:11 GMT\r\n"
		"\r\n");
  write(socket, header, len);
}

int		is_not_num(char *str)
{
  int		i;

  if (str == NULL)
    return (0);
  i = 0;
  while (str[i] != '\0')
    {
      if (str[i] >= '0' || str[i] <= '9' || (str[i] == '.' && (str[i + 1] >= '0' || str[i + 1] <= '9')))
	i++;
      else
	return (1);
      i++;
    }
  return (0);
}

char		get_by_name_dimension_id(char *volume, char *dimension, t_serv_comm *s)
{
  t_vol		*tmp;
  int		i;

  tmp = s->general->volume_first;
  while (tmp != NULL)
    {
      if (strcmp(tmp->path, volume) == 0)
	{
	  i = 0;
	  while (tmp->dim_name[i] != NULL)
	    {
	      if (strcmp(tmp->dim_name[i], dimension) == 0)
		return (i + 48);
	      i++;
	    }
	}
      tmp = tmp->next;
    }
  return (48);
}

void		interpret_header(char *buff, FILE *file, t_serv_comm *s)
{
  int		i;
  int		j;
  char		*volume = NULL;
  char		*dimension = NULL;
  char		*slice = NULL;
  char		*scale = NULL;
  char		*quality = NULL;
  char		*service = NULL;
  char		*square = NULL;
  char		*x = NULL;
  char		*y = NULL;
  char		*x_end = NULL;
  char		*y_end = NULL;
  char		**tmp;
  char		**tmp2;
  char		*line;
  char		comm[400];

  if (strncmp(buff, "GET /?volume=", 13) == 0)
    {
      tmp = serv_str_to_wordtab(buff, '\n');
      line = strdup(tmp[0]);
      i = 0;
      while (tmp[i] != NULL)
	free(tmp[i++]);
      tmp = serv_str_to_wordtab(line, '?');
      line = strdup(tmp[1]);
      i = 0;
      while (tmp[i] != NULL)
	free(tmp[i++]);
      tmp = serv_str_to_wordtab(line, '&');
      i = 0;
      while (tmp[i] != NULL)
	{
	  tmp2 = serv_str_to_wordtab(tmp[i], '=');
	  if (strcmp(tmp2[0], "volume") == 0)               volume = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "dimension") == 0)	    dimension = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "slice") == 0)	    slice = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "scale") == 0)	    scale = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "quality") == 0)	    quality = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "service") == 0)	    service = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "square") == 0)	    square = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "y") == 0)	            y = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "x") == 0)	            x = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "y_end") == 0)	    y_end = strdup(tmp2[1]);
	  else if (strcmp(tmp2[0], "x_end") == 0)	    x_end = strdup(tmp2[1]);
	  j = 0;
	  while (tmp2[j] != NULL)
	    free(tmp2[j++]);
	  i++;
	}
      if (is_not_num(slice) || is_not_num(dimension) || is_not_num(scale) ||
	  is_not_num(quality) || is_not_num(x) || is_not_num(y))
	{
	  fprintf(stderr, "Invalid argumen: non interger\n");
	  return;
	}
      dimension[0] = get_by_name_dimension_id(volume, dimension, s);
      if (service == NULL)
	{
	  quality = serv_str_to_wordtab(quality, ' ')[0];
	  sprintf(comm, "start png %s %i %i %i %i %i %i %s %s %s 1", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, "full");

	}
      else if (strcmp(service, "tiles") == 0)
	{
	  x = serv_str_to_wordtab(x, ' ')[0];
	  sprintf(comm, "start png %s %i %i %i %i %i %i %s %s %s %s %s %s 1", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, service, square, y, x);
	}
      else if (strcmp(service, "images") == 0)
	{
	  x_end = serv_str_to_wordtab(x_end, ' ')[0];
	  sprintf(comm, "start png %s %i %i %i %i %i %i %s %s %s %s %s %s %s 1", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, service, y, x, y_end, x_end);
	}
      printf("comm = %s\n", comm);
      s->general->plug_actions(s->general, comm, file);
    }
}

void		serv_accept_new_connections(t_serv_comm *s)
{
  int		socket;
  struct sockaddr_in	client_addr;
  socklen_t	len;
  FILE		*file;
  char		buff[4096];
  int		l;

  l = 0;
  len = sizeof(client_addr);
  if ((socket = accept(s->sock_serv, (struct sockaddr*)&client_addr,
		       &len)) == -1)
    {
      fprintf(stderr, "Accept Error\n");
      s->state = FAIL;
      return;
    }
  l = read(socket, buff, 4096);
  buff[l] = '\0';
  file = fdopen(socket, "wr");
  write_header(socket);
  interpret_header(buff, file, s);
}

void		reset_set_fd_to_monitor(t_serv_comm *s)
{
  s->bigger_fd = 0;
  FD_ZERO(&(s->rd_fds));
  FD_SET(s->sock_serv, &(s->rd_fds));
  if (s->sock_serv > s->bigger_fd)
    s->bigger_fd = s->sock_serv;
}

void		serv_working_loop(t_serv_comm *s)
{
  s->state = ON;
  while (s->state != OFF && s->state != FAIL)
    {
      reset_set_fd_to_monitor(s);
      if (select((s->bigger_fd + 1), &(s->rd_fds),
		 NULL, NULL, NULL) == -1)
	{
	  fprintf(stderr, "Select Error\n");
	  s->state = FAIL;
	}
      if (FD_ISSET(s->sock_serv, &(s->rd_fds)))
	serv_accept_new_connections(s);
    }
}

int		serv_init_connect(t_serv_comm *s)
{
  int		yes;

  yes = 1;
  if ((s->sock_serv = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      fprintf(stderr, "Socket creation Error\n");
      return (-1);
    }
  memset(&(s->serv_addr), 0, sizeof(s->serv_addr));
  s->serv_addr.sin_family = AF_INET;
  s->serv_addr.sin_addr.s_addr = INADDR_ANY;
  s->serv_addr.sin_port = htons(s->port);

  if (setsockopt(s->sock_serv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  if (bind(s->sock_serv, (struct sockaddr*)&(s->serv_addr),
	   sizeof(s->serv_addr)) < 0)
    {
      fprintf(stderr, "Bind Error\n");
      return (-1);
    }
  listen(s->sock_serv, s->queue_size);
  return (0);
}

void		*init(void *args)
{
  t_args_plug	*a;
  t_serv_comm	*principal;

  a = (t_args_plug*)args;
  principal = malloc(sizeof(*principal));
  a->this->stock = principal;
  return (NULL);
}

void		*start(void *args)
{
  t_args_plug	*a;
  t_serv_comm	*s;

  a = (t_args_plug*)args;
  s = (t_serv_comm*)a->this->stock;
  s->port = atoi(a->commands[0]);
  s->bigger_fd = 0;
  s->general = a->general_info;
  serv_init_connect(s);
  serv_working_loop(s);
  return (NULL);
}

void		*unload(void *args)
{
  return (NULL);
}
