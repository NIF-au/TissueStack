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
#include "core.h"

int		israw(char *path, int existing_fd) {
  int		fd;
  char		check[10];

  memset(check, '\0', 9);

  if (existing_fd <= 0 && ((existing_fd = open(path, O_RDONLY)) == -1)) {
		  perror("Open ");
		  return (-1);
  }
  fd = existing_fd;
  lseek(fd, 0, SEEK_SET);

  if (read(fd, check, 8) == -1)
    {
      perror("Read ");
      return (-1);
    }
  check[9] = '\0';

  if (strcmp(check, "@IaMraW@") != 0) return (0);

  return (fd);
}

void		alloc_info_volume(t_vol *volume)
{
  volume->dimensions = malloc(volume->dim_nb * sizeof(*volume->dimensions));
  volume->starts = malloc(volume->dim_nb * sizeof(*volume->starts));
  volume->steps = malloc(volume->dim_nb * sizeof(*volume->steps));
  volume->size = malloc(volume->dim_nb * sizeof(*volume->size));
  volume->dim_name = malloc(volume->dim_nb * sizeof(*volume->dim_name));
  volume->dim_name_char = malloc((volume->dim_nb + 1) * sizeof(*volume->dim_name_char));
  volume->slice_size = malloc(volume->dim_nb * sizeof(*volume->slice_size));
  volume->dim_offset = malloc(volume->dim_nb * sizeof(*volume->dim_offset));
  volume->raw_format = MINC;	// default to MINC for compatibility
  volume->raw_data_type = UCHAR_8_BIT;	// default to unsigned char for compatibility
}

char		*get_header_len(char *header_magick)
{
  char		tmp[8];
  int		i;
  int		j;

  i = 0;
  while (i < 15)
    {
      if (header_magick[i] == '|')
	break;
      i++;
    }
  i++;
  j = 0;
  while (i < 15 && j < 8)
    {
      if (header_magick[i] == '|')
	break;
      tmp[j] = header_magick[i];
      j++;
      i++;
    }
  if (j >= 8)
    j = 7;
  tmp[j] = '\0';
  return (strdup(tmp));
}

char		**get_array_info(int fd, int *count, int *h_len)
{
  char		header_magick[15];
  char		*header;
  int		magick_len;
  int		header_len;
  char		*string_magick_len;
  char		**tab_info;
  t_string_buffer *buffer = NULL;
  int		len;

  if ((len = read(fd, header_magick, 15)) == -1)
    {
      perror("Read ");
      return (NULL);
    }
  header_magick[len] = '\0';
  string_magick_len = get_header_len(header_magick);
  header_len = atoi(string_magick_len);
  magick_len = strlen(string_magick_len) + 10;

  lseek(fd, magick_len, SEEK_SET);
  header = malloc(header_len * sizeof(*header) + 1);
  if ((len = read(fd, header, header_len)) == -1)
    {
      perror("Read ");
      return (NULL);
    }
  header[len] = '\0';

  buffer = appendToBuffer(buffer, header);
  *count = countTokens(buffer->buffer, '|', '\\');
  tab_info = tokenizeString(buffer->buffer, '|', '\\');

  *h_len = magick_len + header_len;

  // frees
  if (string_magick_len != NULL) free(string_magick_len);
  if (buffer != NULL) {
	  if (buffer->buffer != NULL) free(buffer->buffer);
	  free(buffer);
  }
  if (header != NULL) free(header);

  return (tab_info);
}

char		**get_splitted(char *src, char c, int *count)
{
  char		**dest;
  t_string_buffer *tmp_buff = NULL;

  tmp_buff = appendToBuffer(tmp_buff, src);
  *count = countTokens(tmp_buff->buffer, c, '\\');
  dest = tokenizeString(tmp_buff->buffer, c, '\\');

  // frees
  if (tmp_buff != NULL) {
	  if (tmp_buff->buffer != NULL) free(tmp_buff->buffer);
	  free(tmp_buff);
  }

  return (dest);
}

void		set_dimension_size(t_vol *volume, char *str)
{
  char		**tmp;
  int		count;
  int		i;

  tmp = get_splitted(str, ':', &count);
  if (tmp == NULL)
    {
      volume->size[0] = 0;
      volume->size[1] = 0;
      volume->size[2] = 0;
      return;
    }
  i = 0;
  while (i < count)
    {
      volume->size[i] = atoi(tmp[i]);
      i++;
    }
  i = 0;
  while (tmp[i] != NULL)
    free(tmp[i++]);
  free(tmp);
}

void		set_dimension_start(t_vol *volume, char *str)
{
  char		**tmp;
  int		count;
  int		i;

  tmp = get_splitted(str, ':', &count);
  if (tmp == NULL)
    {
      volume->starts[0] = 0;
      volume->starts[1] = 0;
      volume->starts[2] = 0;
      return;
    }
  i = 0;
  while (i < count)
    {
      volume->starts[i] = atof(tmp[i]);
      i++;
    }
  i = 0;
  while (tmp[i] != NULL)
    free(tmp[i++]);
  free(tmp);
}

void		set_dimension_step(t_vol *volume, char *str)
{
  char		**tmp;
  int		count;
  int		i;

  tmp = get_splitted(str, ':', &count);
  if (tmp == NULL)
    {
      volume->steps[0] = 0;
      volume->steps[1] = 0;
      volume->steps[2] = 0;
      return;
    }
  i = 0;
  while (i < count)
    {
      volume->steps[i] = atof(tmp[i]);
      i++;
    }
  i = 0;
  while (tmp[i] != NULL)
    free(tmp[i++]);
  free(tmp);
}

void		set_dimension_offset(t_vol *volume, char *str, int header_offset)
{
  char		**tmp;
  int		count;
  int		i;

  tmp = get_splitted(str, ':', &count);
  if (tmp == NULL)
    {
      volume->dim_offset[0] = 0;
      volume->dim_offset[1] = 0;
      volume->dim_offset[2] = 0;
      return;
    }
  i = 0;
  while (i < count)
    {
      volume->dim_offset[i] = atoll(tmp[i]) + header_offset;
      i++;
    }
  i = 0;
  while (tmp[i] != NULL)
    free(tmp[i++]);
  free(tmp);
}

void		set_slice_size(t_vol *volume, char *str)
{
  char		**tmp;
  int		count;
  int		i;

  tmp = get_splitted(str, ':', &count);
    if (tmp == NULL)
    {
      volume->slice_size[0] = 0;
      volume->slice_size[1] = 0;
      volume->slice_size[2] = 0;
      return;
    }
  i = 0;
  while (i < count)
    {
      volume->slice_size[i] = atoi(tmp[i]);
      i++;
    }
  i = 0;
  while (tmp[i] != NULL)
    free(tmp[i++]);
  free(tmp);
}

int		raw_volume_init(t_memory_mapping * memory_mappings, t_vol *volume, int fd)
{
  char		**info;
  int		count;
  int		header_len;

  alloc_info_volume(volume);

  if ((info = get_array_info(fd, &count, &header_len)) == NULL)
    return (-1);

  volume->raw_fd = fd;

  if (count < 13)
	  return -1;

  volume->dim_nb = atoi(info[0]);
  set_dimension_size(volume, info[1]);
  set_dimension_start(volume, info[2]);
  set_dimension_step(volume, info[3]);

  volume->dim_name[0] = strdup(info[4]);
  volume->dim_name[1] = strdup(info[5]);
  volume->dim_name[2] = strdup(info[6]);

  volume->dim_name_char[0] = info[7][0];
  volume->dim_name_char[1] = info[8][0];
  volume->dim_name_char[2] = info[9][0];
  volume->dim_name_char[3] = '\0';

  set_slice_size(volume, info[10]);

  volume->slices_max = atoi(info[11]);
  set_dimension_offset(volume, info[12], header_len);

  if (count > 13) // optional info on original format
	  volume->raw_format = atoi(info[13]);
  if (count > 14) // optional info on raw data type
	  volume->raw_data_type = atoi(info[14]);

  volume->raw_data = 1;
  volume->minc_volume = NULL;
  volume->next = NULL;

  free_null_terminated_char_2D_array(info);

  // add memory mapping
  if (memory_mappings != NULL) memory_mappings->add(memory_mappings, volume->path);

  return (0);
}

int		init_volume(t_memory_mapping * memory_mappings, t_vol *volume, char *path)
{
  int		result;
  int		path_len;

  path_len = strlen(path);
  volume->dim_nb = 3;
  volume->path = malloc((path_len + 1) * sizeof(*volume->path));
  volume->path[path_len] = '\0';
  memcpy(volume->path, path, path_len);
  if (volume->path == NULL)
    return (-1);

  volume->color_range_min = 0;
  volume->color_range_max = 255;

  // check if is raw file
  if ((result = israw(volume->path, 0)) == -1)
    return (-1);
  if (result > 0)
    return (raw_volume_init(memory_mappings, volume, result));

  // open the minc file
  if ((result = miopen_volume(volume->path, MI2_OPEN_READ, &volume->minc_volume)) != MI_NOERROR)
    {
      ERROR("Error opening input file: %d.", result);
      return (-1);
    }

  if ((result = miget_volume_dimension_count(volume->minc_volume, 0, 0, &volume->dim_nb)) != MI_NOERROR)
    {
      ERROR("Error getting number of dimensions: %d.", result);
      return (-1);
    }

  alloc_info_volume(volume);

  // get the volume dimensions
  if ((result = miget_volume_dimensions(volume->minc_volume, MI_DIMCLASS_SPATIAL, MI_DIMATTR_ALL,
					MI_DIMORDER_FILE, volume->dim_nb, volume->dimensions)) == MI_ERROR)
    {
      ERROR("Error getting dimensions: %d.", result);
      return (-1);
    }
  // get the size of each dimensions
  if ((result = miget_dimension_sizes(volume->dimensions, volume->dim_nb, volume->size)) != MI_NOERROR)
    {
      ERROR("Error getting dimensions size: %d.", result);
      return (-1);
    }
  if ((result = miget_dimension_starts(volume->dimensions, 0, volume->dim_nb, volume->starts)) != MI_NOERROR)
    {
      ERROR("Error getting dimensions start: %d.", result);
      return (-1);
    }
  if ((result = miget_dimension_separations(volume->dimensions, 0, volume->dim_nb, volume->steps)) != MI_NOERROR)
    {
      ERROR("Error getting dimensions steps: %d.", result);
      return (-1);
    }
  if (miget_dimension_name(volume->dimensions[0], &volume->dim_name[0]) != MI_NOERROR ||
      miget_dimension_name(volume->dimensions[1], &volume->dim_name[1]) != MI_NOERROR ||
      miget_dimension_name(volume->dimensions[2], &volume->dim_name[2]))
    {
      ERROR("Error getting dimensions name.");
      return (-1);
    }
  if (volume->dim_name[0] && volume->dim_name[1] && volume->dim_name[2])
    {
      volume->dim_name_char[0] = volume->dim_name[0][0];
      volume->dim_name_char[1] = volume->dim_name[1][0];
      volume->dim_name_char[2] = volume->dim_name[2][0];
      volume->dim_name_char[3] = '\0';
    }

  // get slices_max
  volume->slices_max = get_slices_max(volume);
  volume->next = NULL;
  return (0);
}

void		*file_actions(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  if (strcmp(a->name, "load") == 0)
    add_volume(a->commands[0], a->general_info);
  else if (strcmp(a->name, "unload") == 0)
    remove_volume(a->commands[0], a->general_info);
  return (NULL);
}

void		add_volume(char *path, t_tissue_stack *t)
{
  t_vol		*tmp = NULL;
  t_vol		*save = NULL;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      save = tmp;
      tmp = tmp->next;
    }
  tmp = malloc(sizeof(*tmp));
  if (init_volume(t->memory_mappings, tmp, path) == 0)
    {
      if (t->volume_first == NULL)
	t->volume_first = tmp;
      else if (save)
	save->next = tmp;
    }
  else
    free(tmp);
}

void		list_volumes(t_tissue_stack *t, char *options)
{
  t_vol		*tmp;

  tmp = t->volume_first;
  if (tmp == NULL)
    printf("No volume in the list\n");
  else
    {
      while (tmp != NULL)
	{
	  printf("\nVolume = %s\n", tmp->path);
	  if (options != NULL && strcmp(options, "--verbose") == 0)
	    {
	      printf("\tMinc Volume address = %p\n", tmp->minc_volume);
	      printf("\tRAW Volume file descriptor = %i\n", tmp->raw_fd);
	      printf("\tDimension Number = %i\n", tmp->dim_nb);
	      printf("\tDimension Name = %s - %s - %s\n", tmp->dim_name[0], tmp->dim_name[1], tmp->dim_name[2]);
	      printf("\tDimension sizes = %u - %u - %u\n", tmp->size[0], tmp->size[1], tmp->size[2]);
	      printf("\tStart value = %g - %g - %g\n", tmp->starts[0], tmp->starts[1], tmp->starts[2]);
	      printf("\tSteps value = %g - %g - %g\n", tmp->steps[0], tmp->steps[1], tmp->steps[2]);
	      printf("\tIs it raw? %s\n", tmp->raw_data == 1 ? "YES" : "NO");
	      printf("\tDimension offsets = %llu - %llu - %llu\n", tmp->dim_offset[0], tmp->dim_offset[1], tmp->dim_offset[2]);
	      printf("\tSlice size (in px) = %i\n", tmp->slice_size[0]);
	    }
	  tmp = tmp->next;
	}
    }
}

t_vol		*check_volume(char *path, t_tissue_stack *t)
{
  t_vol		*v = NULL;

  if ((v = get_volume(path, t)) == NULL)
    {
      add_volume(path, t);
      v = get_volume(path, t);
    }
  return (v);
}

t_vol		*get_volume(char *path, t_tissue_stack *t)
{
  t_vol		*tmp = NULL;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      if (strcmp(path, tmp->path) == 0)
	return (tmp);
      tmp = tmp->next;
    }
  return (NULL);
}

void		remove_volume(char *path, t_tissue_stack *t)
{
  t_vol		*tmp = NULL;
  t_vol		*save = NULL;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      if (strcmp(path, tmp->path) == 0)
	{
	  save->next = tmp->next;
	  free_volume(tmp);
	  break;
	}
      save = tmp;
      tmp = tmp->next;
    }
}

t_vol		*load_volume(t_args_plug * a, char * path)
{
  if (path == NULL) return NULL;

  t_vol * volume = get_volume(path, a->general_info);
  if (volume != NULL) return volume;

  a->this->busy = 1;

  char		volume_load[200];
  sprintf(volume_load, "file load %s", path);

  add_volume(path, a->general_info);
  //  a->general_info->plug_actions(a->general_info, volume_load, NULL);
  volume = get_volume(path, a->general_info);
  /*
  int waitLoops = 0;
  while (volume == NULL && waitLoops < 5) {
    usleep(100000);
    volume = a->general_info->get_volume(path, a->general_info);
    waitLoops++;
    }*/
  a->this->busy = 0;
  if (volume == NULL) {
    ERROR("Failed to load volume: %s", path);
    return NULL;
  }

  return volume;
}

void		free_volume(t_vol *v)
{
  if (v == NULL) return;

  int		i;

  i = 0;
  while (i < v->dim_nb)
	{
	  //if (v->dimensions != NULL && v->dimensions[i] != NULL) mifree_dimension_handle(v->dimensions[i]);
	  if (v->dim_name != NULL && v->dim_name[i] != NULL) free(v->dim_name[i]);
	  i++;
	}
  if (v->dim_name != NULL) free(v->dim_name);
  if (v->dimensions != NULL) free(v->dimensions);

  if (v->starts != NULL) free(v->starts);
  if (v->steps != NULL) free(v->steps);

  if (v->size != NULL) free(v->size);
  if (v->path != NULL) free(v->path);
  if (v->dim_offset != NULL) free(v->dim_offset);
  if (v->slice_size != NULL) free(v->slice_size);
  if (v->dim_name_char != NULL) free(v->dim_name_char);

  miclose_volume(v->minc_volume);

  v->next = NULL;
  free(v);
}

int		get_dim_size(int dim_nb, char * dim_name_char, unsigned int * size, char c)
{
  int		i = 0;

  while (i < dim_nb)
    {
      if (dim_name_char[i] == c)
	return (size[i]);
      i++;
    }
  return (0);
}

void		get_width_height(int *height, int *width, int current_dimension, int dim_nb, char * dim_name_char, unsigned int * size) {
	  if (dim_name_char[current_dimension] == 'x') {
	      *height = get_dim_size(dim_nb, dim_name_char, size, 'z'); //volume->size[Y];
	      *width = get_dim_size(dim_nb, dim_name_char, size, 'y'); //volume->size[Z];
	    }
	  else if (dim_name_char[current_dimension] == 'y')
	    {
	      *height = get_dim_size(dim_nb, dim_name_char, size, 'z');//volume->size[X];
	      *width = get_dim_size(dim_nb, dim_name_char, size, 'x');//volume->size[Z];
	    }
	  else
	    {
	      *height = get_dim_size(dim_nb, dim_name_char, size, 'y');//volume->size[X];
	      *width = get_dim_size(dim_nb, dim_name_char, size, 'x');//volume->size[Y];
	    }
}

void		free_all_volumes(t_tissue_stack *t)
{
  t_vol		*tmp;
  t_vol		*save;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      save = tmp;
      tmp = tmp->next;
      free_volume(save);
    }
}

char		get_by_name_dimension_id(t_vol * vol, char *dimension) {
  int		i;

  if (!dimension || !vol) return (0);

	  i = 0;
	  while (i < vol->dim_nb)
	    {
	      if (strcmp(vol->dim_name[i], dimension) == 0) return (i + 48);
	      i++;
	    }

	  return (48);
}

/*
 * !!!! Orientation correction for MINC and NIFTI !!!!
 * For backwards compatibility this may be still done when tiling on the fly ...
 * IF the raw file has not been created in the compatible format already
 * see RAW header and enum in core.h: FORMAT.GENERIC (3) to identify a ready to use GENERIC RAW
 */
Image * extractSliceDataAtProperOrientation(
		enum RAW_FORMAT original_format,
		enum RAW_TYPE data_type,
		char * dim_name_char,
		int dim,
		void * image_data,
		int width, int height,
		FILE * socketDescriptor) {
	if (dim_name_char == NULL) return NULL;

    ExceptionInfo exception;
    Image		*img = NULL;
    Image		*tmp = NULL;
	ImageInfo	*image_info = NULL;

	// init exception
    GetExceptionInfo(&exception);

    if ((image_info = CloneImageInfo((ImageInfo *)NULL)) == NULL) {
    	dealWithException(&exception, socketDescriptor, NULL, image_info);
    	return NULL;
     }

    if (original_format == MINC &&
  	  ((dim_name_char[0] == 'y' && dim_name_char[1] == 'z' && dim_name_char[2] == 'x' && dim_name_char[dim] == 'x') ||
        (dim_name_char[0] == 'z' && dim_name_char[1] == 'x' && dim_name_char[2] == 'y' && dim_name_char[dim] == 'z') ||
        (dim_name_char[0] == 'x' && dim_name_char[1] == 'z' && dim_name_char[2] == 'y' && (dim_name_char[dim] == 'z' || dim_name_char[dim] == 'y')) ||
        (dim_name_char[0] == 'x' && dim_name_char[1] == 'y' && dim_name_char[2] == 'z') ||
        (dim_name_char[0] == 'y' && dim_name_char[1] == 'x' && dim_name_char[2] == 'z' && (dim_name_char[dim] == 'y' || dim_name_char[dim] == 'x')))) {
    	if ((img = ConstituteImage(height, width, (data_type == RGB_24BIT) ? "RGB" : "I", CharPixel, image_data, &exception)) == NULL) {
      	dealWithException(&exception, socketDescriptor, NULL, image_info);
      	return NULL;
       }

        if ((dim_name_char[0] == 'x' && dim_name_char[1] == 'z' && dim_name_char[2] == 'y' &&
        		(dim_name_char[dim] == 'z' || dim_name_char[dim] == 'y'))) {
  		  tmp = img;
  		  if ((img = RotateImage(img, 90, &exception)) == NULL) {
  	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
  	    	return NULL;
  		  }
  		  DestroyImage(tmp);
        } else {
      	  tmp = img;
      	  if ((img = RotateImage(img, -90, &exception)) == NULL) {
   	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
   	    	return NULL;
      	  }
      	  DestroyImage(tmp);
        }
      } else {
      	if ((img = ConstituteImage(width, height, (data_type == RGB_24BIT) ? "RGB" : "I", CharPixel, image_data, &exception)) == NULL) {
      		dealWithException(&exception, socketDescriptor, NULL, image_info);
  	    	return NULL;
        }
    }
    if ((original_format == NIFTI) ||  (original_format == MINC &&
  	  !((dim_name_char[0] == 'y' && dim_name_char[1] == 'z' && dim_name_char[2] == 'x' && dim_name_char[dim] == 'x') ||
        (dim_name_char[0] == 'z' && dim_name_char[1] == 'x' && dim_name_char[2] == 'y' && dim_name_char[dim] == 'z') ||
        (dim_name_char[0] == 'y' && dim_name_char[1] == 'x' && dim_name_char[2] == 'z' && (dim_name_char[dim] == 'y' ||
        dim_name_char[dim] == 'x')) || (dim_name_char[0] == 'x' && dim_name_char[1] == 'y' && dim_name_char[2] == 'z')))) {
        tmp = img;

        if ((img = FlipImage(img, &exception)) == NULL) {
  	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
  	    	return NULL;
        }
        DestroyImage(tmp);
      }

   	 if ((dim_name_char[0] == 'x' && dim_name_char[1] == 'z' && dim_name_char[2] == 'y')
   			 && (dim_name_char[dim] == 'z' || dim_name_char[dim] == 'y')) {
        tmp = img;
        if ((img = FlopImage(img, &exception)) == NULL) {
  	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
  	    	return NULL;
        }
        DestroyImage(tmp);
      }

    SyncImagePixels(img);

    if (image_info != NULL) DestroyImageInfo(image_info);

    return img;
}

void dealWithException(ExceptionInfo *exception, FILE * socketDescriptor, Image * img, ImageInfo * image_info) {
	char error[200];

	if (exception == NULL) return;

	CatchException(exception);

	if (socketDescriptor == NULL) {
		// log to file
		ERROR("Error: %s",
				exception->description != NULL ? exception->description :
				(exception->reason != NULL ? exception->reason: "N/A"));
		return;
	}

	// compose error message
	sprintf(error,
			"{\"error\": {\"description\": \"Application Exception\", \"message\": \"%s\"}}",
			exception->description != NULL ? exception->description :
					(exception->reason != NULL ? exception->reason: "N/A"));

	// write out error
	write_http_response(socketDescriptor, error, NULL, "application/json");
	fclose(socketDescriptor);

	// clean up
	if (img != NULL) DestroyImage(img);
	if (image_info != NULL) DestroyImageInfo(image_info);
}

void		write_header_into_file(int fd, t_header *h)
{
  char		head[4096];
  char		lenhead[200];
  int		len;

  memset(head, '\0', 4096);
  sprintf(head, "%i|%i:%i:%i|%g:%g:%g|%g:%g:%g|%s|%s|%s|%c|%c|%c|%i:%i:%i|%i|%llu:%llu:%llu|3|2|",
	  h->dim_nb,
	  h->sizes[0], h->sizes[1], h->sizes[2],
	  h->start[0], h->start[1], h->start[2],
	  h->steps[0], h->steps[1], h->steps[2],
	  h->dim_name[0], h->dim_name[1], h->dim_name[2],
	  h->dim_name[0][0], h->dim_name[1][0], h->dim_name[2][0],
	  h->slice_size[0], h->slice_size[1], h->slice_size[2],
	  h->slice_max,
	  (unsigned long long)h->dim_offset[0],
	  (unsigned long long)h->dim_offset[1],
	  (unsigned long long)h->dim_offset[2]);
  len = strlen(head);
  memset(lenhead, '\0', 200);
  sprintf(lenhead, "@IaMraW@|%i|", len);
  write(fd, lenhead, strlen(lenhead));
  write(fd, head, len);
}
