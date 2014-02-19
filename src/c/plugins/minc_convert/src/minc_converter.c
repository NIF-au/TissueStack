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
#include "minc_converter.h"


/*
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
*/

void		*init(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  LOG_INIT(a);
  InitializeMagick("./");
  INFO("Minc Converter init");

  return (NULL);
}

void  		*start(void *args)
{
  int		fd = 0;
  t_vol		*minc_volume;
  t_header	*header;
  t_args_plug	*a;
  char		*id_percent;
  unsigned int	dimension;
  unsigned int	slice;
  unsigned long long off = 0L;
  int		i = 0;
  char		*command_line;

  a = (t_args_plug *)args;

  prctl(PR_SET_NAME, "TS_MINC_CON");
  if ((a->commands[3] != NULL && a->commands[4] != NULL && a->commands[5] != NULL) ||
      (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0
       && a->commands[3] != NULL && strlen(a->commands[3]) == 16))
    {
      if (a->commands[2] != NULL)
	{
	  dimension = atoi(a->commands[2]);
	  slice = atoi(a->commands[3]);
	  if ((fd = open(a->commands[1], (O_CREAT | O_APPEND | O_RDWR), 0666)) == -1)
	    {
	      ERROR("Open Failed");
	      return (NULL);
	    }
	  minc_volume = init_get_volume_from_minc_file(a->commands[0]);
	  header = create_header_from_minc_struct(minc_volume);
	  while (i < dimension)
	    {
	      off += header->dim_offset[i];
	      i++;
	    }
	  if (slice != 0)
	    off += header->slice_size[i] * (slice - 1);
	  lseek(fd, off, SEEK_SET);
	  if (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0 && a->commands[3] != NULL && strlen(a->commands[3]) == 16)
	    dim_loop(fd, minc_volume->dim_nb, minc_volume, a->general_info,
		     a->commands[3], -1, -1);
	  else
	    dim_loop(fd, minc_volume->dim_nb, minc_volume, a->general_info,
		     a->commands[5], (slice - 1), dimension);

	  close(fd);
	  return (NULL);
	}
    }
  else
    {
      if ((fd = open(a->commands[1], (O_CREAT | O_TRUNC | O_RDWR), 0666)) == -1)
	{
	  ERROR("Open Failed");
	  return (NULL);
	}
      if (chmod(a->commands[1], 0644) == -1)
	ERROR("Chmod failed");
      minc_volume = init_get_volume_from_minc_file(a->commands[0]);
      header = create_header_from_minc_struct(minc_volume);
      write_header_into_file(fd, header);
      if (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0)
	{
	  command_line = array_2D_to_array_1D(a->commands);
	  a->general_info->percent_init(get_nb_total_slices_to_do(minc_volume), &id_percent, a->commands[0], "1", a->commands[1], command_line, a->general_info);
	  if (a->box != NULL)
	    {
	      if (write(*((int*)a->box), id_percent, 16) < 0)
		ERROR("Write Error");
	    }
	  a->general_info->tasks->add_to_queue(id_percent, a->general_info);
	  free(command_line);
	  close(fd);
	  return (NULL);
	}
      dim_loop(fd, minc_volume->dim_nb, minc_volume, a->general_info, id_percent, -1, -1);
    }
  if (fd != 0)
    {
      if (close(fd) == -1)
	{
	  ERROR("Close failed");
	  return (NULL);
	}
    }
  return (NULL);
}

void		*unload(void *args)
{
    DestroyMagick();
	INFO("Minc Convert Plugin: Unloaded");
	return (NULL);
}
