#include "minc_extract_png.h"

void		*percentage(void *args)
{
  t_png_args	*a;
  float       	percent;

  a = (t_png_args *)args;
  percent = 0;
  while (percent < 100)
    {
      pthread_mutex_lock(&(a->p->lock));
      percent = (100 / (float)a->info->total_slices_to_do) * a->info->slices_done;
      //  printf("\r%.2f - %u Slices done on %u", percent, a->info->slices_done, a->info->total_slices_to_do);
      pthread_mutex_unlock(&(a->p->lock));
      usleep(1000);
    }
  //  printf("\r%.2f - %u Slices done on %u\n", (float)100, a->info->slices_done, a->info->total_slices_to_do);
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
  unsigned int	nb_slices;
  int		**dim_start_end;

  i = 0;
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
  a->this->stock = (void*)png_args;
  return (NULL);
}

void		*start(void *args)
{
  t_png_extract	*png_args;
  t_args_plug	*a;
  t_vol		*volume;

  a = (t_args_plug *)args;
  a->this->busy = 1;
  if ((volume = a->general_info->get_volume(a->commands[0], a->general_info)) == NULL)
    {
      write(2, "MINC Volume not found\n", strlen("MINC Volume not found\n"));
      a->this->busy = 0;
      return (NULL);
    }
  png_args = (t_png_extract *)a->this->stock;
  png_args->dim_start_end = generate_dims_start_end(volume,
  						    atoi(a->commands[1]), atoi(a->commands[2]),
						    atoi(a->commands[3]), atoi(a->commands[4]),
						    atoi(a->commands[5]), atoi(a->commands[6]));
  png_args->total_slices_to_do = get_total_slices_to_do(volume, png_args->dim_start_end);
  png_creation_lunch(volume, atoi(a->commands[7]), a->general_info->tp, png_args, a->this, (FILE*)a->box);
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
