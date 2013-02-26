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
	  a->dim_start_end[i][1] != -1)
	{
	  if (strcmp(a->service, "full") == 0)
	    {
	      if (a->dim_start_end[i][1] == 0 && a->dim_start_end[i][0] == 0)
		slices = volume->size[i];
	      else
		slices = a->dim_start_end[i][1] - a->dim_start_end[i][0];
	      count += slices;
	    }
	  else
	    {
	      if (a->h_position == -1 && a->w_position == -1)
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
	    }
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
  short		exit = 0;

  a = (t_image_args *)args;
  volume = a->volume;

  i = 0;
  if (a->info->percentage == 1)
    prctl(PR_SET_NAME, "TS_TILING");
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
  while (i < volume->dim_nb && exit == 0)
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
      if (a->info->percent == 1)
	exit = a->general_info->is_percent_paused_cancel(a->info->id_percent, a->general_info);
    }
  //  a->general_info->percent_add(10, a->info->id_percent, a->general_info);
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
  char	        *hyperslab = NULL;
  int		w_max_iteration;
  int		h_max_iteration;
  int		save_h_position = a->info->h_position;
  int		save_w_position = a->info->w_position;
  short 	free_hyperslab = -1;
  short		exit = 0;

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
  while (current_slice < max && exit == 0)
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
	  while (a->info->start_h <= h_max_iteration && exit == 0)
	    {
	      a->info->start_w = 0;
	      while (a->info->start_w <= w_max_iteration && exit == 0)
		{
		  a->info->h_position = a->info->start_h;
		  a->info->w_position = a->info->start_w;
		  print_image(hyperslab, volume, current_dimension, current_slice, width, height, a);
		  a->info->start_w++;
		  a->general_info->percent_add(1, a->info->id_percent, a->general_info);
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
      DEBUG("SLICE == %i ==> %i", current_slice, current_dimension);
      exit = a->general_info->is_percent_paused_cancel(a->info->id_percent, a->general_info);
      current_slice++;

      // restore original positions
      a->info->h_position = save_h_position;
      a->info->w_position = save_w_position;
    }
  start[current_dimension] = 0;
  if (free_hyperslab > 0) free(hyperslab);
}
