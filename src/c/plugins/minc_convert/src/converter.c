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

void		dim_loop(int		fd,
			 int		dimensions_nb,
			 t_vol		*volume,
			 t_tissue_stack	*t,
			 char *id_percent)
{
  int		dim = 0;
  int		slice = 0;
  int		this_slice = 0;
  int		size;
  char		*hyperslab;
  int		i;
  unsigned long		*start;
  long unsigned int	*count;

  DEBUG("dim_loop");

  start = malloc(volume->dim_nb * sizeof(*start));
  count = malloc(volume->dim_nb * sizeof(*count));
  start[0] = start[1] = start[2] = 0;
  i = 0;
  while (i < volume->dim_nb)
    {
      count[i] = volume->size[i];
      i++;
    }
  while (dim < dimensions_nb)
    {
      size = (dim == 0 ? (volume->size[2] * volume->size[1]) :
	      (dim == 1 ? (volume->size[0] * volume->size[2]) : (volume->size[0] * volume->size[1])));
      hyperslab = malloc(size * sizeof(*hyperslab));
      slice = volume->size[dim];
      this_slice = 0;
      count[dim] = 1;
      while (this_slice < slice)
	{
	  start[dim] = this_slice;
	  memset(hyperslab, '\0', size);
	  miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE, start, count, hyperslab);
	  write(fd, hyperslab, size);
	  INFO("Slice = %i - dim = %i", this_slice, dim);
	  this_slice++;
	  t->percent_add(1, id_percent, t);
	}
      start[dim] = 0;
      count[dim] = volume->size[dim];
      dim++;
      free(hyperslab);
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
      ERROR("Error opening input file: %d.", result);
      return (NULL);
    }

  if ((result = miget_volume_dimension_count(volume->minc_volume, 0, 0, &volume->dim_nb)) != MI_NOERROR)
    {
      ERROR("Error getting number of dimensions: %d.", result);
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
      ERROR("Error getting dimensions: %d.", result);
      return (NULL);
    }
  // get the size of each dimensions
  if ((result = miget_dimension_sizes(volume->dimensions, volume->dim_nb, volume->size)) != MI_NOERROR)
    {
      ERROR("Error getting dimensions size: %d.", result);
      return (NULL);
    }
  if ((result = miget_dimension_starts(volume->dimensions, 0, volume->dim_nb, volume->starts)) != MI_NOERROR)
    {
      ERROR("Error getting dimensions start: %d.", result);
      return (NULL);
    }
  if ((result = miget_dimension_separations(volume->dimensions, 0, volume->dim_nb, volume->steps)) != MI_NOERROR)
    {
      ERROR("Error getting dimensions steps: %d.", result);
      return (NULL);
    }
  if (miget_dimension_name (volume->dimensions[0], &volume->dim_name[0]) != MI_NOERROR ||
      miget_dimension_name (volume->dimensions[1], &volume->dim_name[1]) != MI_NOERROR ||
      miget_dimension_name (volume->dimensions[2], &volume->dim_name[2]))
    {
      ERROR("Error getting dimensions name.");
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
      DEBUG("----- %llu", (unsigned long long)h->dim_offset[i]);
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
  sprintf(head, "%i|%i:%i:%i|%g:%g:%g|%g:%g:%g|%s|%s|%s|%c|%c|%c|%i:%i:%i|%i|%llu:%llu:%llu|",
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

void		*init(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  LOG_INIT(a);
  return (NULL);
}

void  		*start(void *args)
{
  int		fd;
  t_vol		*minc_volume;
  t_header	*header;
  t_args_plug	*a;
  char		*id_percent;

  a = (t_args_plug *)args;

  prctl(PR_SET_NAME, "TS_MINC_CON");
  if ((fd = open(a->commands[1], (O_CREAT | O_TRUNC | O_RDWR))) == -1)
    {
      ERROR("Open Failed");
      return (NULL);
    }
  minc_volume = init_get_volume_from_minc_file(a->commands[0]);

  a->general_info->percent_init(get_nb_total_slices_to_do(minc_volume), &id_percent, a->general_info);
  if (write(*((int*)a->box), id_percent, 10) < 0)
    ERROR("Open Error");

  header = create_header_from_minc_struct(minc_volume);
  write_header_into_file(fd, header);
  dim_loop(fd, minc_volume->dim_nb, minc_volume, a->general_info, id_percent);
  if (close(fd) == -1)
    {
      ERROR("Close failed");
      return (NULL);
    }
  if (chmod(a->commands[1], 0644) == -1)
    ERROR("Chmod failed");

  return (NULL);
}

void		*unload(void *args)
{
  return (NULL);
}
