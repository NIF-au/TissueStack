#include "minc_extract_png.h"

unsigned int		get_slices_max(t_vol *volume)
{
  // get the larger number of slices possible
  printf("volume_size =\nX = %i\nY = %i\nZ = %i\n", volume->size[X], volume->size[Y], volume->size[Z]);
  if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[X]))
    {
      if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[Y]))
	return (volume->size[X] * volume->size[Y]);
    }
  else if ((volume->size[Z] * volume->size[X]) > (volume->size[Z] * volume->size[Y]))
    return (volume->size[Z] * volume->size[X]);
  return (volume->size[Z] * volume->size[Y]);
}

void            *get_all_slices_of_all_dimensions(void *args)
{
  t_vol		*volume;
  t_png_args	*a;
  int           i;
  unsigned long *start;
  long unsigned int *count;


  a = (t_png_args *)args;
  volume = a->volume;
  i = 0;
  // init start and count variable
  count = malloc(volume->dim_nb * sizeof(*count));
  while (i < volume->dim_nb)
    {
      count[i] = volume->size[i];
      i++;
    }
  start = malloc(volume->dim_nb * sizeof(*start));
  start[X] = start[Y] = start[Z] = 0; // start to 0 = first slice
  // loop all dimensions
  i = 0;
  while (i < volume->dim_nb)
    {
      // check if the dimension need to be ignored
      if (a->dim_start_end[i][0] != -1 &&
	  a->dim_start_end[i][1] != -1)
	{
	  // set count variable to 1 for the current dimension to indicate the step (here 1 == 1 slice per time)
	  count[i] = 1;
	  get_all_slices_of_one_dimension(volume, start, i, count, a);
	  count[i] = volume->size[i];
	}
      i++;
    }
  // some free variable
  free(a);
  free(start);
  free(count);
  return (NULL);
}

void            get_all_slices_of_one_dimension(t_vol *volume, unsigned long *start, int current_dimension,
						long unsigned int *count, t_png_args *a)
{
  unsigned int	current_slice;
  unsigned int	max;
  int		width;
  int		height;
  double        *hyperslab;
  //int        *hyperslab;

  // allocation of a hyperslab (portion of the file, can be 1 slice or 1 demension...)
  hyperslab =  malloc(volume->slices_max * sizeof(*hyperslab));
  // set the first slice extracted
  current_slice = (unsigned int)a->dim_start_end[current_dimension][0];
  // set the last slice extracted
  max = (unsigned int)(a->dim_start_end[current_dimension][1] == 0 ?
		       volume->size[current_dimension] : a->dim_start_end[current_dimension][1]);
  // init height and width compare to the dimension
  get_width_height(&height, &width, current_dimension, volume);
  // loop all the slices
  while (current_slice < max)
    {
      memset(hyperslab, 0, (volume->slices_max * sizeof(*hyperslab)));
      start[current_dimension] = current_slice;
      // get the data of 1 slice
      pthread_mutex_lock(&(a->p->lock));
      /*printf("volume = %p\nstart = %p (%lu - %lu - %lu)\ncount = %p (%lu - %lu - %lu)\nhyperslab =  %p\n",
	volume->minc_volume, start, start[0], start[1], start[2], count, count[0], count[1], count[2], hyperslab);*/
      miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_DOUBLE, start, count, hyperslab);
      pthread_mutex_unlock(&(a->p->lock));
      // print png
      print_png(hyperslab, volume, current_dimension, current_slice, width, height, a->file);
      pthread_mutex_lock(&(a->p->lock));
      a->info->slices_done++;
      pthread_mutex_unlock(&(a->p->lock));
      current_slice++;
    }
  start[current_dimension] = 0;
  free(hyperslab);
}
