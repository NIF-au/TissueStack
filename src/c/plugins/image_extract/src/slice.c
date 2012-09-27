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

int		get_nb_blocks_percent(t_image_extract *a, t_vol *volume)
{
  int		i = 0;
  int		count = 0;
  int		height;
  int		width;
  int		slices;
  int		h_tiles;
  int		w_tiles;
  int		tiles_per_slice;

  while (i < volume->dim_nb)
    {
      if (a->dim_start_end[i][0] != -1 &&
	  a->dim_start_end[i][1] != -1 &&
	  a->h_position == -1 &&
	  a->w_position == -1)
	{
	  get_width_height(&height, &width, i, volume);
	  h_tiles = (height * a->scale) / a->square_size;
	  w_tiles = (width * a->scale) / a->square_size;

	  if ((height % a->square_size) != 0)
	    h_tiles++;
	  if ((width % a->square_size) != 0)
	    w_tiles++;
	  if (a->dim_start_end[i][1] == 0 && a->dim_start_end[i][0] == 0)
	    slices = volume->size[i];
	  else
	    slices = a->dim_start_end[i][1] - a->dim_start_end[i][0];

	  tiles_per_slice = h_tiles * w_tiles;

	  count += (slices * tiles_per_slice);
	}
      i++;
    }
  return (count);
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

void		get_raw_data_hyperslab(t_memory_mapping * memory_mappings, t_vol *volume, int dim, int slice, char *hyperslab)
{
  unsigned long long int offset;

  if (volume->raw_data == 1)
    {
      offset = (volume->dim_offset[dim] + (unsigned long long int)((unsigned long long int)volume->slice_size[dim] * (unsigned long long int)slice));
      /*
      if (memory_mappings != NULL) {
    	  char * data = memory_mappings->get(memory_mappings, volume->path);
    	  if (data != NULL) {
    		  memcpy(hyperslab, &data[offset], volume->slice_size[dim]);
			  return;
			  }
			  }*/

      // plan B: read in a regular fashion
      lseek(volume->raw_fd, offset, SEEK_SET);
      read(volume->raw_fd, hyperslab, volume->slice_size[dim]);
    }
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
  char *buff = NULL;

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose(a->file);
	return;
  }

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
      if (volume->raw_data != 1)
	{
	  pthread_mutex_lock(&(a->p->lock));
	  miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE, start, count, hyperslab);
	  pthread_mutex_unlock(&(a->p->lock));
	}
      else
	get_raw_data_hyperslab(a->general_info->memory_mappings, volume, current_dimension, current_slice, hyperslab);
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
		  a->general_info->percent_add(1, a->info->id_percent, a->general_info);
		  a->general_info->percent_get(&buff, a->info->id_percent, a->general_info);
		}
	      a->info->start_h++;
	    }
	}
      else {
	print_image(hyperslab, volume, current_dimension, current_slice, width, height, a);
      }

      /*      char *buff = NULL;
      a->general_info->percent_get(&buff, a->info->id_percent, a->general_info->percent);
      FATAL("========> %s%%", buff);*/

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
  free(hyperslab);
}
