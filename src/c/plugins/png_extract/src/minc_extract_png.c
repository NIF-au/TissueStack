#include "minc_extract_png.h"

void		get_percent(FILE *file, t_png_extract *a)
{
  char		buff[20];

  if (a->percent > 100)
    a->percent = 100;
  sprintf(buff, "%.2f", a->percent);
  if (file != NULL)
    {
      fwrite(buff, 1, strlen(buff), file);
      return;
    }
  printf("%.2f%%\n", a->percent);
}

void		*percentage(void *args)
{
  t_png_args	*a;

  a = (t_png_args *)args;
  a->info->percent = 0;
  while (a->info->percent < 100)
    {
      pthread_mutex_lock(&(a->info->mut));
      if (a->info->percent < 100)
	pthread_cond_wait(&(a->info->cond), &(a->info->mut));
      a->info->percent = (100 / (float)a->info->total_slices_to_do) * a->info->slices_done;
      pthread_mutex_unlock(&(a->info->mut));
      //usleep(1000);
    }
  a->this->busy = 0;
  free(a);
  return (NULL);
}

unsigned int	get_total_slices_to_do(t_vol *v, int **dim_start_end)
{
  int		i;
  int		count;

  count = 0;
  i = 0;
  while (i < 3)
    {
      count += dim_start_end[i][1] - dim_start_end[i][0];
      i++;
    }
  return (count);
}

int		**generate_dims_start_end(t_vol *v, int sx, int ex, int sy,
					  int ey, int sz, int ez)
{
  int		i;
  int		**dim_start_end;

  i = 0;
  dim_start_end = malloc(3 * sizeof(*dim_start_end));
  while (i < 3)
    {
      dim_start_end[i] = malloc(2 * sizeof(*dim_start_end[i]));
      i++;
    }
  dim_start_end[0][0] = sx;
  dim_start_end[0][1] = (ex == 0 ? v->size[0] : ex);
  dim_start_end[1][0] = sy;
  dim_start_end[1][1] = (ey == 0 ? v->size[1] : ey);
  dim_start_end[2][0] = sz;
  dim_start_end[2][1] = (ez == 0 ? v->size[2] : ez);
  return (dim_start_end);
}

int		**generate_dims_start_end_thread(t_vol *v, int dim, int start, int end)
{
  int		i;
  int		**dim_start_end;

  i = 0;
  dim_start_end = malloc(3 * sizeof(*dim_start_end));
  while (i < 3)
    {
      dim_start_end[i] = malloc(2 * sizeof(*dim_start_end[i]));
      dim_start_end[i][1] = -1;
      dim_start_end[i][0] = -1;
      i++;
    }
  dim_start_end[dim][0] = start;
  if (end != 0)
    dim_start_end[dim][1] = end;
  else
    dim_start_end[dim][1] = v->size[dim];
  return (dim_start_end);
}

t_png_args	*create_args_thread(t_thread_pool *p, t_vol *vol, t_png_extract *png_general, t_plugin *this, FILE *sock)
{
  t_png_args   *args;

  args = malloc(sizeof(*args));
  args->volume = vol;
  args->p = p;
  args->info = png_general;
  args->this = this;
  args->file = sock;
  return (args);
}

void		lunch_percent_display(t_thread_pool *p, t_vol *vol, t_png_extract *png_general, t_plugin *this)
{
  t_png_args   *args;

  args = create_args_thread(p, vol, png_general, this, NULL);
  (*p->add)(percentage, (void *)args, p);
}

void		png_creation_lunch(t_vol *vol, int step, t_thread_pool *p, t_png_extract *png_general, t_plugin *this, FILE *sock)
{
  t_png_args	*args;
  unsigned int	i;
  unsigned int	j;
  unsigned int	nb_slices = 0;
  int		**dim_start_end;

  i = 0;

  // infinite loop paranoia
  if (step <= 0) {
	  step = 1;
  }

  dim_start_end = png_general->dim_start_end;
  lunch_percent_display(p, vol, png_general, this);
  while (i < 3)
    {
      if (dim_start_end[i][0] != -1 && dim_start_end[i][1] != -1)
	{
	  j = 0;
	  nb_slices = ((dim_start_end[i][1] == 0 ? vol->size[i] : dim_start_end[i][1]) - dim_start_end[i][0]);
	  while (j < nb_slices)
	    {
	      args = create_args_thread(p, vol, png_general, this, sock);
	      if ((dim_start_end[i][0] + step) <= dim_start_end[i][1])
		args->dim_start_end = generate_dims_start_end_thread(vol, i, dim_start_end[i][0], dim_start_end[i][0] + step);
	      else
		args->dim_start_end = generate_dims_start_end_thread(vol, i, dim_start_end[i][0], dim_start_end[i][1]);
	      (*p->add)(get_all_slices_of_all_dimensions, (void *)args, p);
	      j += step;
	      dim_start_end[i][0] += step;
	    }
	}
      i++;
    }
}

int		check_input(char **in)
{
  int		i;
  int		j;

  i = 1;
  while (in[i + 1] != NULL)
    {
      if (strcmp(in[i], "-1") != 0 && strcmp(in[i], "tiles") != 0 &&
	  strcmp(in[i], "images") != 0 && strcmp(in[i], "full") != 0)
	{
	  j = 0;
	  while (in[i][j] != '\0')
	    {
	      if (in[i][j] != '.')
		{
		  if (in[i][j] < '0' || in[i][j] > '9')
		    {
		      fprintf(stderr, "Error invalid argument : %s\n", in[i]);
		      return (1);
		    }
		}
	      j++;
	    }
	}
      i++;
    }
  return (0);
}

int		check_range(int **d, t_vol *v)
{
  if (v == NULL) printf("Volume is NULL\n");

  int		i;

  i = 0;
  while (i < v->dim_nb)
    {
      if (d[i][0] != -1 || d[i][1] != -1)
	{
	  if (d[i][0] > v->size[i] || d[i][1] > v->size[i] || d[i][0] < 0 || d[i][1] < 0)
	    return (1);
	}
      i++;
    }
  return (0);
}

void		*init(void *args)
{
  t_png_extract	*png_args;
  t_args_plug	*a;

  a = (t_args_plug *)args;
  png_args = malloc(sizeof(*png_args));
  png_args->total_slices_to_do = 0;
  png_args->slices_done = 0;
  png_args->step = 0;
  png_args->dim_start_end = NULL;
  pthread_mutex_init(&png_args->mut, NULL);
  pthread_cond_init(&png_args->cond, NULL);
  a->this->stock = (void*)png_args;
  InitializeMagick("./");
  /*
  free(a->name);
  int i =0;
  while (a->commands[i] != NULL)
    free(a->commands[i++]);
  free(a->commands);
  free(a->path);*/
  a->destroy(a);
  return (NULL);
}

void		*start(void *args)
{
  t_png_extract	*png_args;
  t_args_plug	*a;
  t_vol		*volume;
  int		step;
  char		volume_load[200];
  FILE		*socketDescriptor;

  a = (t_args_plug *)args;

  if ((volume = a->general_info->get_volume(a->commands[0], a->general_info)) == NULL)
    {
      a->this->busy = 1;
      sprintf(volume_load, "file load %s", a->commands[0]);
      a->general_info->plug_actions(a->general_info, volume_load, NULL);
      int waitLoops = 0;
      while (volume == NULL && waitLoops < 5) {
    	  usleep(100000);
    	  volume = a->general_info->get_volume(a->commands[0], a->general_info);
    	  waitLoops++;
      }
      a->this->busy = 0;
      if (volume == NULL) {
    	  printf("Failed to load volume: %s\n", a->commands[0] == NULL ? "no file given" : a->commands[0]);
   		  return NULL;
      }
    }

  // please don't move this line up above the loading of the volume
  socketDescriptor = (FILE*)a->box;

  png_args = (t_png_extract *)a->this->stock;
  if (strcmp(a->commands[1], "percent") == 0)
    {
      get_percent((FILE*)a->box, png_args);
      a->this->busy = 0;
      return (NULL);
    }
  if (check_input(a->commands))
    {
      a->this->busy = 0;
      return (NULL);
    }
  png_args->dim_start_end = generate_dims_start_end(volume,
  						    atoi(a->commands[1]), atoi(a->commands[2]),
						    atoi(a->commands[3]), atoi(a->commands[4]),
						    atoi(a->commands[5]), atoi(a->commands[6]));
  if (check_range(png_args->dim_start_end, volume))
    {
      fprintf(stderr, "Slice out of volume range\n");
      a->this->busy = 0;
      return (NULL);
    }
  png_args->square_size = -1;
  png_args->w_position = 0;
  png_args->h_position = 0;
  png_args->w_position_end = -1;
  png_args->h_position_end = -1;
  png_args->scale = (float)atof(a->commands[7]);
  png_args->quality = atoi(a->commands[8]);
  png_args->service = a->commands[9];

  if (strcmp(png_args->service, "tiles") == 0)
    {
      png_args->square_size = atoi(a->commands[10]);
      png_args->h_position = atoi(a->commands[11]);
      png_args->w_position = atoi(a->commands[12]);
      png_args->start_h = atoi(a->commands[11]);
      png_args->start_w = atoi(a->commands[12]);
      if (a->commands[14] != NULL)
	png_args->root_path = strdup(a->commands[14]);
    }
  else if (strcmp(png_args->service, "images") == 0)
    {
      png_args->h_position = atoi(a->commands[10]);
      png_args->w_position = atoi(a->commands[11]);
      png_args->start_h = atoi(a->commands[10]);
      png_args->start_w = atoi(a->commands[11]);
      png_args->h_position_end = atoi(a->commands[12]);
      png_args->w_position_end = atoi(a->commands[13]);
      if (a->commands[15] != NULL)
	png_args->root_path = strdup(a->commands[15]);
    }
  else
    {
      if (strcmp(png_args->service, "full") != 0)
	{
	  fprintf(stderr, "Undefined kind. Please choose 'tiles' or 'images'\n");
	  a->this->busy = 0;
	  return (NULL);
	}
      else
	{
	  if (a->commands[11] != NULL)
	    png_args->root_path = strdup(a->commands[11]);
	}
    }
  png_args->done = 0;
  png_args->total_slices_to_do = get_total_slices_to_do(volume, png_args->dim_start_end);

  step = (strcmp(png_args->service, "tiles") == 0 ? atoi(a->commands[13]) : (strcmp(png_args->service, "full") == 0 ? atoi(a->commands[10]) : atoi(a->commands[14])));
  png_creation_lunch(volume, step, a->general_info->tp, png_args, a->this, socketDescriptor);
  return (NULL);
}

void		*unload(void *args)
{
  t_png_extract	*png_args;
  t_args_plug	*a;

  a = (t_args_plug *)args;
  a->this->busy = 1;
  png_args = (t_png_extract *)a->this->stock;
  free(png_args->dim_start_end[0]);
  free(png_args->dim_start_end[1]);
  free(png_args->dim_start_end[2]);
  free(png_args->dim_start_end);
  free(png_args);
  free(a->name);
  free(a->path);
  a->this->busy = 0;
  return (NULL);
}
