#include "serv.h"

unsigned int		get_slices_max(t_vol *volume)
{
  // get the larger number of slices possible
  if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[X]))
    {
      if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[Y]))
	return (volume->size[X] * volume->size[Y]);
    }
  else if ((volume->size[Z] * volume->size[X]) > (volume->size[Z] * volume->size[Y]))
    return (volume->size[Z] * volume->size[X]);
  return (volume->size[Z] * volume->size[Y]);
}

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
  int		i = 0;
  int		j = 0;
  int wordCount = 0;
  char		**dest = NULL;

  // preliminary checks
  if (buff == NULL) {
	  return NULL;
  }

  // if word count is 0 => good bye
  wordCount = serv_word_count(buff, c);
  if (wordCount == 0) {
	  return NULL;
  }

  dest = malloc((wordCount + 1) * sizeof(*dest));

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

  // terminate 2D array with NULL
  dest[j] = NULL;

  return (dest);
}

int		is_not_num(char *str)
{
  int		i;

  if (str == NULL)
    return (0);
  i = 0;
  while (str[i] != '\0' && i < strlen(str))
    {
      if (str[i] >= '0' || str[i] <= '9' || (str[i] == '.' && (str[i + 1] && (str[i + 1] >= '0' || str[i + 1] <= '9'))))
	i++;
      else
	return (1);
    }
  return (0);
}

char		get_by_name_dimension_id(t_vol * vol, char *dimension, t_serv_comm *s)
{
  int		i;

  if (!dimension || !vol || !s) return (0);

	  i = 0;
	  while (vol->dim_name[i] != NULL)
	    {
	      if (strcmp(vol->dim_name[i], dimension) == 0) return (i + 48);
	      i++;
	    }

	  return (48);
}

char		*serv_copy_check_clean_string_from_tab(char **tab)
{
  char		*str;

  str = strdup(tab[1]);
  str[strlen(str)] = '\0';
  str = serv_str_to_wordtab(str, ' ')[0];
  return (str);
}

void		interpret_header(t_args_plug * a,  char *buff, FILE *file, t_serv_comm *s)
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
  char		**tmp = NULL;
  char		**tmp2 = NULL;
  char		*line = NULL;
  char		*image_type = NULL;
  char		*colormap_name = NULL;
  char		*id = NULL;
  char		*time = NULL;
  char		*contrast_min = NULL;
  char		*contrast_max = NULL;
  char		comm[500];

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
	  if (strcmp(tmp2[0], "volume") == 0)		volume = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "dimension") == 0)	dimension = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "slice") == 0)	slice = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "scale") == 0)	scale = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "quality") == 0)	quality = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "service") == 0)	service = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "square") == 0)	square = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "y") == 0)		y = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "x") == 0)		x = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "y_end") == 0)	y_end = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "x_end") == 0)	x_end = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "colormap") == 0)	colormap_name = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "image_type") == 0)	image_type = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "min") == 0)	contrast_min = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "max") == 0)	contrast_max = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "id") == 0)	id = serv_copy_check_clean_string_from_tab(tmp2);
	  else if (strcmp(tmp2[0], "timestamp") == 0)	time = serv_copy_check_clean_string_from_tab(tmp2);
	  j = 0;
	  while (tmp2[j] != NULL)
	    free(tmp2[j++]);
	  i++;
	}
      if (is_not_num(slice) || is_not_num(dimension) || is_not_num(scale) ||
	  is_not_num(quality) || is_not_num(x) || is_not_num(y))
	{
	  fprintf(stderr, "Invalid argument: non interger\n");
	  return;
	}

      t_vol		*vol = load_volume(a, volume);

      if (vol == NULL) {
          write_http_header(file, "500 Server Error", "png");
    	  fclose(file);
    	  return;
      }

      if ((dimension[0] = get_by_name_dimension_id(vol, dimension, s)) == 0) return;

      if (service == NULL)
	{
	  sprintf(comm, "start image %s %i %i %i %i %i %i %s %s %s %s %s %s %s 1 %s %s", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, "full", image_type, (colormap_name == NULL ? "NULL" : colormap_name),
		  (contrast_min == NULL ? "0" : contrast_min), (contrast_max == NULL ? "0" : contrast_max),
		  id != NULL ? id : "0", time != NULL ? time : "0");
	}
      else if (strcmp(service, "tiles") == 0)
	{
	  sprintf(comm, "start image %s %i %i %i %i %i %i %s %s %s %s %s %s %s %s %s %s 1 %s %s", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, service, image_type, square, y, x, (colormap_name == NULL ? "NULL" : colormap_name),
		  (contrast_min == NULL ? "0" : contrast_min), (contrast_max == NULL ? "0" : contrast_max),
		  id != NULL ? id : "0", time != NULL ? time : "0");
	}
      else if (strcmp(service, "images") == 0)
	{
	  sprintf(comm, "start image %s %i %i %i %i %i %i %s %s %s %s %s %s %s %s %s %s %s 1 %s %s", volume,
		  (dimension[0] == '0' ? atoi(slice) : -1),
		  (dimension[0] == '0' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '1' ? atoi(slice) : -1),
		  (dimension[0] == '1' ? (atoi(slice) + 1) : -1),
		  (dimension[0] == '2' ? atoi(slice) : -1),
		  (dimension[0] == '2' ? (atoi(slice) + 1) : -1),
		  scale, quality, service, image_type, y, x, y_end, x_end, (colormap_name == NULL ? "NULL" : colormap_name),
		  (contrast_min == NULL ? "0" : contrast_min), (contrast_max == NULL ? "0" : contrast_max),
		  id != NULL ? id : "0", time != NULL ? time : "0");
	}

      s->general->tile_requests->add(s->general->tile_requests, id, time);

      if (s->general->tile_requests->is_expired(s->general->tile_requests, id, time)) {
          write_http_header(file, "408 Request Timeout", image_type);
    	  close(fileno(file));
    	  return;
      }

      s->general->plug_actions(s->general, comm, file);
    }
}

void		serv_accept_new_connections(t_args_plug * a, t_serv_comm *s)
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
  interpret_header(a, buff, file, s);
}

void		reset_set_fd_to_monitor(t_serv_comm *s)
{
  s->bigger_fd = 0;
  FD_ZERO(&(s->rd_fds));
  FD_SET(s->sock_serv, &(s->rd_fds));
  if (s->sock_serv > s->bigger_fd)
    s->bigger_fd = s->sock_serv;
}

void		serv_working_loop(t_args_plug * a, t_serv_comm *s)
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
	serv_accept_new_connections(a, s);
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
  listen(s->sock_serv, 75);
  return (0);
}

void		*init(void *args)
{
  t_args_plug	*a;
  t_serv_comm	*principal;

  a = (t_args_plug*)args;
  principal = malloc(sizeof(*principal));
  a->this->stock = principal;

  LOG_INIT(a);
  INFO("Server Plugin: Started");

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
  serv_working_loop(a, s);
  return (NULL);
}

void		*unload(void *args)
{
  destroy_plug_args((t_args_plug*)args);
  return (NULL);
}
