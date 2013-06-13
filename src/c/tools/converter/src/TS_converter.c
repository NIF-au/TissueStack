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

#include "converter.h"

unsigned int            get_slices_max(t_vol *volume)
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

void		dim_loop(int fd, int dimensions_nb, t_vol *volume,
			 t_tissue_stack *t, char *id_percent,
			 int slice_resume, int dimension_resume)
{
  int		dim = 0;
  int		slice = 0;
  int		this_slice = 0;
  int		size;
  char		*hyperslab;
  int		i;
  unsigned long		*start;
  long unsigned int	*count;
  short			cancel = 0;

  if (dimension_resume > -1)
    dim = dimension_resume;
  start = malloc(volume->dim_nb * sizeof(*start));
  count = malloc(volume->dim_nb * sizeof(*count));
  start[0] = start[1] = start[2] = 0;
  i = 0;
  while (i < volume->dim_nb)
    {
      count[i] = volume->size[i];
      i++;
    }
  printf("Dimensions size: size[0] ==> %i | size[1] ==> %i | size[2] ==> %i\n\n", (int)volume->size[0], (int)volume->size[1], (int)volume->size[2]);
  while (dim < dimensions_nb && cancel == 0)
    {
      size = (dim == 0 ? (volume->size[2] * volume->size[1]) :
	      (dim == 1 ? (volume->size[0] * volume->size[2]) : (volume->size[0] * volume->size[1])));
      hyperslab = malloc(size * sizeof(*hyperslab));
      slice = volume->size[dim];
      if (slice_resume != -1)
	{
	  this_slice = slice_resume;
	  slice_resume = -1;
	}
      else
	this_slice = 0;
      count[dim] = 1;
      while (this_slice < slice && cancel == 0)
	{
	  start[dim] = this_slice;
	  memset(hyperslab, '\0', size);
	  miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE, (misize_t*)start, (misize_t*)count, hyperslab);
	  write(fd, hyperslab, size);
	  printf("Slice = %i / %i - dim = %i\r", this_slice, (int)volume->size[dim], dim);
	  fflush(stdout);
	  this_slice++;
	}
      start[dim] = 0;
      count[dim] = volume->size[dim];
      dim++;
      free(hyperslab);
      printf("                                                                                                                                          \r");
    }
}

t_vol		*init_get_volume_from_minc_file(char *path)
{
  t_vol		*volume;
  int		result;

  volume = malloc(sizeof(*volume));
  volume->path = path;
  volume->dim_nb = 3;
  if (volume->path == NULL)
    return (NULL);
  // open the minc file
  if ((result = miopen_volume(volume->path, MI2_OPEN_READ, &volume->minc_volume)) != MI_NOERROR)
    {
      fprintf(stderr, "Error opening input file: %d.\n", result);
      return (NULL);
    }

  if ((result = miget_volume_dimension_count(volume->minc_volume, 0, 0, &volume->dim_nb)) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting number of dimensions: %d.\n", result);
      return (NULL);
    }

  volume->dimensions = malloc(volume->dim_nb * sizeof(*volume->dimensions));
  volume->starts = malloc(volume->dim_nb * sizeof(*volume->starts));
  volume->steps = malloc(volume->dim_nb * sizeof(*volume->steps));
  volume->size = malloc(volume->dim_nb * sizeof(*volume->size));
  volume->dim_name = malloc(volume->dim_nb * sizeof(*volume->dim_name));

  // get the volume dimensions
  if ((result = miget_volume_dimensions(volume->minc_volume, MI_DIMCLASS_SPATIAL, MI_DIMATTR_ALL,
					MI_DIMORDER_FILE, volume->dim_nb, volume->dimensions)) == MI_ERROR)
    {
      fprintf(stderr, "Error getting dimensions: %d.\n", result);
      return (NULL);
    }
  // get the size of each dimensions

  if ((result = miget_dimension_size(volume->dimensions[0], &volume->size[0])) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting dimensions size: %d.\n", result);
      return (NULL);
    }

  if ((result = miget_dimension_size(volume->dimensions[1], &volume->size[1])) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting dimensions size: %d.\n", result);
      return (NULL);
    }

  if ((result = miget_dimension_size(volume->dimensions[2], &volume->size[2])) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting dimensions size: %d.\n", result);
      return (NULL);
    }

  if ((result = miget_dimension_starts(volume->dimensions, 0, volume->dim_nb, volume->starts)) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting dimensions start: %d.\n", result);
      return (NULL);
    }
  if ((result = miget_dimension_separations(volume->dimensions, 0, volume->dim_nb, volume->steps)) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting dimensions steps: %d.\n", result);
      return (NULL);
    }
  if (miget_dimension_name (volume->dimensions[0], &volume->dim_name[0]) != MI_NOERROR ||
      miget_dimension_name (volume->dimensions[1], &volume->dim_name[1]) != MI_NOERROR ||
      miget_dimension_name (volume->dimensions[2], &volume->dim_name[2]))
    {
      fprintf(stderr, "Error getting dimensions name.\n");
      return (NULL);
    }
  // get slices_max
  volume->slices_max = get_slices_max(volume);
  volume->next = NULL;
  return (volume);
}

t_header	*create_header_from_minc_struct(t_vol *minc_volume)
{
  t_header	*h;
  int		i;
  int		j;

  h = malloc(sizeof(*h));

  h->dim_nb = minc_volume->dim_nb;

  h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
  h->start = malloc(h->dim_nb * sizeof(*h->start));
  h->steps = malloc(h->dim_nb * sizeof(*h->steps));
  h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
  h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
  h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));

  h->slice_max = minc_volume->slices_max;

  i = 0;
  while (i < h->dim_nb)
    {
      h->sizes[i] = minc_volume->size[i];
      h->start[i] = minc_volume->starts[i];
      h->steps[i] = minc_volume->steps[i];
      h->dim_name[i] = strdup(minc_volume->dim_name[i]);

      h->slice_size[i] = 1;
      j = 0;
      while (j < h->dim_nb)
	{
	  if (j != i)
	    h->slice_size[i] *= minc_volume->size[j];
	  j++;
	}
      i++;
    }

  h->dim_offset[0] = 0;
  i = 1;
  while (i < h->dim_nb)
    {
      h->dim_offset[i] = (unsigned long long)(h->dim_offset[i - 1] + (unsigned long long)((unsigned long long)h->slice_size[i - 1] * (unsigned long long)h->sizes[i - 1]));
      i++;
    }
  return (h);
}

void		write_header_into_file(int fd, t_header *h)
{
  char		head[4096];
  char		lenhead[200];
  int		len;

  memset(head, '\0', 4096);
  sprintf(head, "%i|%i:%i:%i|%g:%g:%g|%g:%g:%g|%s|%s|%s|%c|%c|%c|%i:%i:%i|%i|%llu:%llu:%llu|1|",
	  h->dim_nb,
	  h->sizes[0], h->sizes[1], h->sizes[2],
	  h->start[0], h->start[1], h->start[2],
	  h->steps[0], h->steps[1], h->steps[2],
	  h->dim_name[0], h->dim_name[1], h->dim_name[2],
	  h->dim_name[0][0], h->dim_name[1][0], h->dim_name[2][0],
	  h->slice_size[0], h->slice_size[1], h->slice_size[2],
	  h->slice_max,
	  (unsigned long long)h->dim_offset[0], (unsigned long long)h->dim_offset[1], (unsigned long long)h->dim_offset[2]);
  len = strlen(head);
  memset(lenhead, '\0', 200);
  sprintf(lenhead, "@IaMraW@|%i|", len);
  write(fd, lenhead, strlen(lenhead));
  write(fd, head, len);
}

int		get_nb_total_slices_to_do(t_vol *volume)
{
  int		i = 0;
  int		count = 0;

  while (i < volume->dim_nb)
    {
      count += volume->size[i];
      i++;
    }
  return (count);
}

int		main(int ac, char **av)
{
  int		fd = 0;
  t_vol		*minc_volume;
  t_header	*header;
  char		*raw_volume;
  char		*minc_path;

  if (ac < 3)
    {
      printf("Usage: ./mnc2raw  [minc volume path] [raw volume path]\n");
      return (-1);
    }

  minc_path = av[1];
  raw_volume = av[2];
  if ((fd = open(raw_volume, (O_CREAT | O_TRUNC | O_RDWR), 0666)) == -1)
    {
      fprintf(stderr, "Open Failed\n");
      return (-1);
    }
  if (chmod(raw_volume, 0644) == -1)
    fprintf(stderr, "Chmod failed\n");
  minc_volume = init_get_volume_from_minc_file(minc_path);
  header = create_header_from_minc_struct(minc_volume);
  write_header_into_file(fd, header);
  dim_loop(fd, minc_volume->dim_nb, minc_volume, NULL, NULL, -1, -1);
  if (fd != 0)
    {
      if (close(fd) == -1)
	{
	  fprintf(stderr, "Close failed\n");
	  return (-1);
	}
    }
  return (0);
}
