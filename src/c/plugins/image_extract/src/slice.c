#include "image_extract.h"

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

void            *get_all_slices_of_all_dimensions(void *args)
{
  t_vol		*volume;
  t_image_args	*a;
  int           i;
  unsigned long *start;
  long unsigned int *count;

  a = (t_image_args *)args;
  volume = a->volume;
  i = 0;
  // copy path as well
  volume->path = strdup(a->volume->path);

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
  free_image_args(a);
  if (start != NULL) free(start);
  if (count != NULL) free(count);
  return (NULL);
}

char *		get_raw_data_hyperslab(t_memory_mapping * memory_mappings, t_vol *volume, int dim, int slice, short * free_hyperslab)
{
  unsigned long long int offset;
  char * hyperslab = NULL;

  offset = (volume->dim_offset[dim] + (unsigned long long int)((unsigned long long int)volume->slice_size[dim] * (unsigned long long int)slice));
  if (memory_mappings != NULL) {
	  hyperslab = memory_mappings->get(memory_mappings, volume->path);
	  if (hyperslab != NULL)  return &hyperslab[offset];
	  printf("Could not do it!!!\n");
  }

  // plan B: read in a regular fashion
  hyperslab = malloc(volume->slices_max * sizeof(*hyperslab));
  *free_hyperslab = 1;
  lseek(volume->raw_fd, offset, SEEK_SET);
  read(volume->raw_fd, hyperslab, volume->slice_size[dim]);

 return hyperslab;
}

void            get_all_slices_of_one_dimension(t_vol *volume, unsigned long *start, int current_dimension,
						long unsigned int *count, t_image_args *a)
{
  unsigned int	current_slice;
  unsigned int	max;
  int		width;
  int		height;
  char	        *hyperslab;
  int		w_max_iteration;
  int		h_max_iteration;
  int		save_h_position = a->info->h_position;
  int		save_w_position = a->info->w_position;
  short 	free_hyperslab = -1;

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose(a->file);
	return;
  }

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
      start[current_dimension] = current_slice;
      // get the data of 1 slice
      if (volume->raw_data != 1)
	{
      free_hyperslab = 1;
	  // allocation of a hyperslab (portion of the file, can be 1 slice or 1 demension...)
	  hyperslab =  malloc(volume->slices_max * sizeof(*hyperslab));
      memset(hyperslab, 0, (volume->slices_max * sizeof(*hyperslab)));

      pthread_mutex_lock(&(a->p->lock));
	  miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE, start, count, hyperslab);
	  pthread_mutex_unlock(&(a->p->lock));
	}
      else
	hyperslab = get_raw_data_hyperslab(a->general_info->memory_mappings, volume, current_dimension, current_slice, &free_hyperslab);
      // print image
      if (a->info->h_position == -1 && a->info->w_position == -1)
	{
	  a->info->h_position = 0;
	  a->info->start_h = 0;
	  a->info->w_position = 0;
	  a->info->start_w = 0;

	  h_max_iteration = (height * a->info->scale) / a->info->square_size;
	  w_max_iteration = (width * a->info->scale) / a->info->square_size;

	  while (a->info->start_h <= h_max_iteration)
	    {
	      a->info->start_w = 0;
	      while (a->info->start_w <= w_max_iteration)
		{
		  a->info->h_position = a->info->start_h;
		  a->info->w_position = a->info->start_w;

		  print_image(hyperslab, volume, current_dimension, current_slice, width, height, a);
		  a->info->start_w++;
		}
	      a->info->start_h++;
	    }
	}
      else {
	print_image(hyperslab, volume, current_dimension, current_slice, width, height, a);
      }

      pthread_mutex_lock(&(a->p->lock));
      a->info->slices_done++;
      pthread_mutex_unlock(&(a->p->lock));
      pthread_cond_signal(&(a->info->cond));
      current_slice++;

      // restore original positions
      a->info->h_position = save_h_position;
      a->info->w_position = save_w_position;
    }
  start[current_dimension] = 0;
  if (free_hyperslab > 0) free(hyperslab);
}
