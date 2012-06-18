#include "serv.h"

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

int		count_len_word_header(char *buff, int position, char sep)
{
  int		i;

  i = position;
  while (buff[position] != sep && buff[position] != '\r' &&
	 buff[position] != '\n' && buff[position] != '\0')
    position++;
  return (position - i);
}

char		*str_n_copy_from_position(char *buff, int position, int len)
{
  int		i;
  char		*dest;

  i = 0;
  dest = malloc(len * sizeof(*dest));
  while (buff[position] != '\0' && i < len)
    {
      dest[i] = buff[position];
      i++;
      position++;
    }
  dest[i] = '\0';
  return (dest);
}

int		str_n_cmp_from_position(char *s1, char *s2, int position, int len)
{
  int		i;

  i = 0;
  while (s1[position] != '\0' && s2[i] != '\0' && i < len)
    {
      if (s1[position] != s2[i])
	return (-1);
      position++;
      i++;
    }
  return (0);
}

int		is_not_num(char *str)
{
  int		i;

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

void		interpret_header(char *buff, FILE *file, t_serv_comm *s)
{
  int		i;
  int		tmp;
  int		pos;
  char		*volume;
  char		*dimension;
  char		*slice;
  char		*scale;
  char		*quality;
  char		*service;
  char		*square;
  char		*x;
  char		*y;
  char		*x_end;
  char		*y_end;
  char		test[400];

  if (strncmp(buff, "GET /?volume=", 13) == 0)
    {
      i = count_len_word_header(buff, 0, '?');
      tmp = count_len_word_header(buff, 0, '=');
      if (str_n_cmp_from_position(buff, "volume", i + 1, tmp - i) != 0)
	return;
      i = count_len_word_header(buff, tmp, '&');
      volume = str_n_copy_from_position(buff, tmp + 1, i - 1);
      pos = tmp + i + 1;
      tmp = count_len_word_header(buff, pos, '=');
      if (str_n_cmp_from_position(buff, "dimension", pos, tmp) != 0)
	return;
      pos += tmp;
      i = count_len_word_header(buff, pos, '&');
      dimension = str_n_copy_from_position(buff, pos + 1, i - 1);
      pos += i + 1;
      tmp = count_len_word_header(buff, pos, '=');
      if (str_n_cmp_from_position(buff, "slice", pos, tmp) != 0)
	return;
      pos += tmp;
      i = count_len_word_header(buff, pos, '&');
      slice = str_n_copy_from_position(buff, pos + 1, i - 1);
      pos += i + 1;
      tmp = count_len_word_header(buff, pos, '=');
      if (str_n_cmp_from_position(buff, "scale", pos, tmp) != 0)
	return;
      pos += tmp;
      i = count_len_word_header(buff, pos, '&');
      scale = str_n_copy_from_position(buff, pos + 1, i - 1);
      pos += i + 1;
      tmp = count_len_word_header(buff, pos, '=');
      if (str_n_cmp_from_position(buff, "quality", pos, tmp) != 0)
	return;
      pos += tmp;
      i = count_len_word_header(buff, pos, '&');
      quality = str_n_copy_from_position(buff, pos + 1, i - 1);

      pos += i + 1;
      tmp = count_len_word_header(buff, pos, '=');
      if (str_n_cmp_from_position(buff, "service", pos, tmp) != 0)
	return;
      pos += tmp;
      i = count_len_word_header(buff, pos, '&');
      service = str_n_copy_from_position(buff, pos + 1, i - 1);

      if (strcmp(service, "tiles") == 0)
	{
	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "square", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, '&');
	  square = str_n_copy_from_position(buff, pos + 1, i - 1);
	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "y", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, '&');
	  y = str_n_copy_from_position(buff, pos + 1, i - 1);
	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "x", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, ' ');
	  x = str_n_copy_from_position(buff, pos + 1, i - 1);
	  if (is_not_num(square))
	    {
	      fprintf(stderr, "Invalid argumen: non interger\n");
	      return;
	    }
	}
      else if (strcmp(service, "images") == 0)
	{

	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "y", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, '&');
	  y = str_n_copy_from_position(buff, pos + 1, i - 1);
	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "x", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, '&');
	  x = str_n_copy_from_position(buff, pos + 1, i - 1);
	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "y_end", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, '&');
	  y_end = str_n_copy_from_position(buff, pos + 1, i - 1);
	  pos += i + 1;
	  tmp = count_len_word_header(buff, pos, '=');
	  if (str_n_cmp_from_position(buff, "x_end", pos, tmp) != 0)
	    return;
	  pos += tmp;
	  i = count_len_word_header(buff, pos, ' ');
	  x_end = str_n_copy_from_position(buff, pos + 1, i - 1);
	  if (is_not_num(x_end) || is_not_num(y_end))
	    {
	      fprintf(stderr, "Invalid argumen: non interger\n");
	      return;
	    }
	}

      if (is_not_num(slice) || is_not_num(dimension) || is_not_num(scale) ||
	  is_not_num(quality) || is_not_num(x) || is_not_num(y))
	{
	  fprintf(stderr, "Invalid argumen: non interger\n");
	  return;
	}
      if (strcmp(service, "tiles") == 0)
	{
	  sprintf(test, "start png %s %i %i %i %i %i %i %s %s %s %s %s %s 1", volume,
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
	  sprintf(test, "start png %s %i %i %i %i %i %i %s %s %s %s %s %s %s 1", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, service, y, x, y_end, x_end);
	}
      (*s->general->plug_actions)(s->general, test, file);
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
