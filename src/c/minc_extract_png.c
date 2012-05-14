#include "minc_extract_png.h"

void		*percentage(void *args)
{
  t_arguments	*a;
  float       	percent;
  
  a = (t_arguments *)args;
  percent = 0;
  while (percent < 100)
    {
      pthread_mutex_lock(&(a->p->lock));
      percent = (100 / (float)a->volume->total_slices_to_do) * a->volume->slices_done;
      printf("\r%.2f - %u Slices done on %u", percent, a->volume->slices_done, a->volume->total_slices_to_do);
      pthread_mutex_unlock(&(a->p->lock));
      usleep(1000);
    }
  printf("\r%.2f - %u Slices done on %u\n", (float)100, a->volume->slices_done, a->volume->total_slices_to_do);
  free(a);
  pthread_cond_broadcast(&(a->p->condvar_main));
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

void		lunch_percent_display(t_thread_pool *p, t_vol *vol)
{
  t_arguments	*args;
  
  args = malloc(sizeof(*args));
  args->opt = NULL;
  args->volume = vol;
  args->p = p;
  thread_pool_add_task(percentage, (void *)args, p);
}

t_thread_pool	*thread_lunch(t_vol *vol, int **dim_start_end, int step, int workers)
{
  t_arguments	*args;
  t_thread_pool	*p;
  unsigned int	i;		
  unsigned int	j;
  unsigned int	nb_slices;
 
  i = 0;
  p = malloc(sizeof(*p));
  thread_pool_init(p, workers + 1);
  lunch_percent_display(p, vol);
  while (i < 3)
    {
      if (dim_start_end[i][0] != -1 && dim_start_end[i][1] != -1)
	{
	  j = 0;
	  nb_slices = ((dim_start_end[i][1] == 0 ? vol->size[i] : dim_start_end[i][1]) - dim_start_end[i][0]); 
	  while (j < nb_slices)
	    {
	      args = malloc(sizeof(*args));
	      args->opt = malloc(sizeof(*args->opt));
	      args->volume = vol;
	      args->p = p;
	      if ((dim_start_end[i][0] + step) <= dim_start_end[i][1])
		args->opt->dim_start_end = generate_dims_start_end_thread(vol, i, dim_start_end[i][0], dim_start_end[i][0] + step);
	      else
		args->opt->dim_start_end = generate_dims_start_end_thread(vol, i, dim_start_end[i][0], dim_start_end[i][1]);
	      thread_pool_add_task(get_all_slices_of_all_dimensions, (void *)args, p);
	      j += step;
	      dim_start_end[i][0] += step;
	    }
	}
      i++;
    }
  return (p);
}

void		lunch_png_creation(t_vol *vol)
{
  t_arguments	*args;

  args = malloc(sizeof(*args));
  args->opt = malloc(sizeof(*args->opt));
  args->volume = vol;
  args->opt->dim_start_end = generate_dims_start_end(vol, 0, 0, -1, -1, -1, -1);
  get_all_slices_of_all_dimensions((void*)args);
}

void		init_prog(t_vol *volume, char *path_arg)
{
  // set the number of dimensions of the volume
  volume->opt = malloc(sizeof(*volume->opt));
  volume->dim_nb = 3;
  // Allocation of all the volume component needed (dimensions, sizes and path of the file)
  volume->dimensions = malloc(volume->dim_nb * sizeof(*volume->dimensions));
  volume->size = malloc(volume->dim_nb * sizeof(*volume->size));
  volume->path = malloc((strlen(path_arg) + 1) * sizeof(*volume->path));
  volume->slices_done = 0;
  volume->total_slices_to_do = 0;
  memcpy(volume->path, path_arg, strlen(path_arg));
}

int		main(int argc, char **argv) {
  t_vol		volume;
  int		result;
  //  double	cosine[3];
  int		**d_s_e;
  t_thread_pool	*p;

  // initialisation of some variable
  init_prog(&volume, argv[1]);
  // open the minc file 
  if ((result = miopen_volume(argv[1], MI2_OPEN_READ, &volume.minc_volume)) != MI_NOERROR) {
    fprintf(stderr, "Error opening input file: %d.\n", result);
  }
  // get the volume dimensions
  if ((result = miget_volume_dimensions(volume.minc_volume, MI_DIMCLASS_SPATIAL, MI_DIMATTR_ALL,
					MI_DIMORDER_FILE, volume.dim_nb, volume.dimensions)) == MI_ERROR)
    fprintf(stderr, "Error getting dimensions: %d.\n", result);
  // get the size of each dimensions
  if ((result = miget_dimension_sizes(volume.dimensions, volume.dim_nb, volume.size)) != MI_NOERROR)
    fprintf(stderr, "Error getting dimensiosn size: %d.\n", result);
  //printf("Volume sizes: %u, %u , %u\n", volume.size[X], volume.size[Y], volume.size[Z]);
  // get slices_max
  volume.slices_max = get_slices_max(&volume);

  /*			Cosine
  miget_dimension_cosines(volume.dimensions[0], cosine);
  printf("%g - %g - %g\n", cosine[0], cosine[1], cosine[2]);
  miget_dimension_cosines(volume.dimensions[1], cosine);
  printf("%g - %g - %g\n", cosine[0], cosine[1], cosine[2]);
  miget_dimension_cosines(volume.dimensions[2], cosine);
  printf("%g - %g - %g\n", cosine[0], cosine[1], cosine[2]);
  */

  d_s_e = generate_dims_start_end(&volume, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]),
				  atoi(argv[6]), atoi(argv[7]));  
  volume.total_slices_to_do = get_total_slices_to_do(&volume, d_s_e);

  //  printf("Total slices to do = %u\n", volume.total_slices_to_do);

  // lunch threads
  
  volume.step = atoi(argv[8]);
  
  p = thread_lunch(&volume, d_s_e, atoi(argv[8]), atoi(argv[9]));

  // free all the stuff mallocked
  
  pthread_mutex_lock(&p->lock_main);
  pthread_cond_wait(&(p->condvar_main), &(p->lock_main));
  pthread_mutex_unlock(&(p->lock));
  thread_pool_destroy(p);

  free(volume.dimensions);
  free(volume.size);
  free(volume.path);
  return (0);
}

