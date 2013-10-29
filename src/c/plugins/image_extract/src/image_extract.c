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

int		get_size_colormap(float **colormap)
{
  int		i;

  i = 0;
  while (colormap[i][0] != 99)
    i++;
  return (i);
}

void 		alloc_and_init_colormap_space_from_lookup(float **new_colormap, float **source)
{
	int i=0;

	// initialize with the original gray value i.e. index
    while (i < 256)
	{
	  new_colormap[i] = malloc(3 * sizeof(*new_colormap[i]));
	  new_colormap[i][0] = new_colormap[i][1] = new_colormap[i][2] = i;
	  i++;
	}

    //now use the color lookup we have
    i = 0;
    while (source[i] != NULL) {
      int j = source[i][0];
  	  new_colormap[j][0] = source[i][1];
  	  new_colormap[j][1] = source[i][2];
  	  new_colormap[j][2] = source[i][3];

  	  //INFO("[%i] => %f | %f | %f \n", j, source[i][1], source[i][2], source[i][3]);

  	  i++;
    }
}

void		alloc_and_init_colormap_space_from_src(float **new_colormap, float **source)
{
	short valueRangeRow = 1;
	float * offsetRGB = malloc(3*sizeof(*offsetRGB));
	offsetRGB[0] = offsetRGB[1] = offsetRGB[2] = 0;
	float valueRangeStart = source[valueRangeRow - 1][0] * 255;
	float valueRangeEnd = source[valueRangeRow][0] * 255;
	unsigned short index = 0;
	unsigned short rgb=1;
	float valueRangeDelta = 0;

	for (index = 0; index < 256 ; index++) {
		// continuous to discrete mapping
		// first check if we are within desired range up the present end,
		// if not => increment row index to move on to next range
		if (index > valueRangeEnd) {
			valueRangeRow++;
			valueRangeStart = valueRangeEnd;
			valueRangeEnd = source[valueRangeRow][0] * 255;
			offsetRGB[0] = source[valueRangeRow-1][1];
			offsetRGB[1] = source[valueRangeRow-1][2];
			offsetRGB[2] = source[valueRangeRow-1][3];
		}
		valueRangeDelta = valueRangeEnd - valueRangeStart;

		// iterate over RGB channels

		new_colormap[index] = malloc(3 * sizeof(*new_colormap[index]));
		for (rgb = 1; rgb < 4 ; rgb++) {
			float rgbRangeStart = source[valueRangeRow-1][rgb];
			float rgbRangeEnd = source[valueRangeRow][rgb];
			float rgbRangeDelta = rgbRangeEnd - rgbRangeStart;
			float rangeRemainder = fmodf((float)index,valueRangeDelta);
			if (rangeRemainder == 0 && rgbRangeDelta != 0 && rgbRangeDelta != 1 && valueRangeEnd == 255) {
				offsetRGB[rgb-1] += rgbRangeDelta;
			}
			float rangeRatio = (rgbRangeDelta * rangeRemainder / valueRangeDelta) + offsetRGB[rgb-1];

			new_colormap[index][rgb-1] = round(rangeRatio * 255);
		}
	}
}

char		*str_n_cpy(char *str, int position, int len)
{
  char		*dest;
  int		i;

  i = 0;
  dest = malloc((len + 1) * sizeof(*dest));
  while (i < len)
    {
      dest[i] = str[i + position];
      i++;
    }
  dest[i] = '\0';
  return (dest);
}

int		img_word_count(char *buff, char c)
{
  int		i;
  int		count;

  i = 0;
  count = 0;
  if (buff[0] != c && buff[0] != '\0')
    count++;
  while (buff[i] != '\0')
    {
      if ((buff[i] == c && buff[i + 1] != c ) && buff[i + 1] != '\0')
	count++;
      i++;
    }
  return (count);
}

int		img_letter_count(char *buff, int position, char c)
{
  int		i;

  i = 0;
  while (buff[position + i] != c && buff[position + i] != '\0')
    i++;
  return (i);
}

char		**img_str_to_wordtab(char *buff, char c)
{
  int		i = 0;
  int		j = 0;
  int wordCount = 0;
  char		**dest = NULL;

  // preliminary checks
  if (buff == NULL) {
	  return NULL;
  }

  // if word count is 0 => good bye
  wordCount = img_word_count(buff, c);
  if (wordCount == 0) {
	  return NULL;
  }

  dest = malloc((wordCount + 1) * sizeof(*dest));

  while (buff[i] != '\0')
    {
      if (buff[i] != c && buff[i] != '\0')
	{
	  dest[j] = str_n_cpy(buff, i, img_letter_count(buff, i, c));
	  j++;
	  i += img_letter_count(buff, i, c);
	}
      if (buff[i] != '\0')
	i++;
    }

  // terminate 2D array with NULL
  dest[j] = NULL;

  return (dest);
}

float		get_float(char *src, int start, int end, int len)
{
  char		*tmp;
  int		index = 0;
  float		result = 0;

  tmp = malloc(((end - start) + 1) * sizeof(*tmp));
  while (start <= end && start < len)
    {
      if (src[start] == ' ' || src[start] == '\t' || src[start] == '\n')
	break;
      if ((src[start] >= '0' && src[start] <= '9') || src[start] == '.')
	tmp[index] = src[start];
      else
	tmp[index] = '0';
      start++;
      index++;
    }
  tmp[index] = '\0';
  result = atof(tmp);
  free(tmp);
  return (result);
}

float		**get_colormap_from_lookup_file(char *path) {
	  int		fd = 0;
	  int		len = 1;
	  char		*buff = NULL;
	  float		**color = NULL;
	  float		colormap_tmp[256][4];
	  int		c_row_index = 0;
	  int		number_of_lines = 0;
	  int		c_column_index = 0;
	  int		j = 0;

	  char		**lines;
	  char		**values;

	  fd = open(path, O_RDONLY);
	  buff = malloc(4096 * sizeof(*buff));
	  while (1)
	    {
	      memset(buff, '\0', 4096);
		  len = read(fd, buff, 4096-1);
		  if (len <= 0) break;
		  buff[len] = '\0';
		  j=0;
		  while (j<len && buff[j] != '\0')
		    {
		      if (buff[j] != ' ' && (buff[j] < '0' || buff[j] > '9') &&
			  buff[j] != '.' && buff[j] != '\n')
			buff[j] = ' ';
		      j++;
		    }
		  lines = img_str_to_wordtab(buff, '\n');
		  c_row_index = 0;
		  while (lines != NULL && lines[c_row_index] != NULL)
		    {
		      values = img_str_to_wordtab(lines[c_row_index], ' ');
		      c_column_index = 0;
		      while (values[c_column_index] != NULL)
			{
			  colormap_tmp[number_of_lines+c_row_index][c_column_index] = atof(values[c_column_index]);
			  c_column_index++;
			}

		      free_null_terminated_char_2D_array(values);
		      c_row_index++;
		    }
		  number_of_lines += c_row_index;
	      free_null_terminated_char_2D_array(lines);
	  }
	  if (fd > 0) close(fd);
	  free(buff);

	  color = malloc((number_of_lines + 1) * sizeof(*color));
	  j=0;
	  while (j < number_of_lines) {
		  color[j] = malloc(4 * sizeof(*color[j]));
		  color[j][0] = colormap_tmp[j][0];
		  color[j][1] = colormap_tmp[j][1];
		  color[j][2] = colormap_tmp[j][2];
		  color[j][3] = colormap_tmp[j][3];
		  j++;
	  }
	  // terminate with NULL
	  color[number_of_lines] = NULL;

	  return color;
}

float		**get_colormap_from_file(char *path)
{
  int		fd = 0;
  int		len = 1;
  char		*buff = NULL;
  float		**color = NULL;
  float		colormap_tmp[256][4];
  int		c_row_index = 0;
  int		c_column_index = 0;
  int		j = 0;
  //  int		k = -1;
  int		flag = 0;
  char		**lines;
  char		**values;

  fd = open(path, O_RDONLY);

  buff = malloc(8096 * sizeof(*buff));
  while (len > 0)
    {
      flag = 0;
      memset(buff, '\0', 8096);
      if ((len = read(fd, buff, 8096)) > 0)
	{
	  buff[len] = '\0';
	  while (j < len && buff[j] != '\0')
	    {
	      if (buff[j] != ' ' && (buff[j] < '0' || buff[j] > '9') &&
		  buff[j] != '.' && buff[j] != '\n')
		buff[j] = ' ';
	      j++;
	    }
	  lines = img_str_to_wordtab(buff, '\n');
	  while (lines[c_row_index] != NULL)
	    {
	      values = img_str_to_wordtab(lines[c_row_index], ' ');
	      c_column_index = 0;
	      while (values[c_column_index] != NULL)
		{
		  colormap_tmp[c_row_index][c_column_index] = atof(values[c_column_index]);
		  c_column_index++;
		}
	      free_null_terminated_char_2D_array(values);
	      c_row_index++;
	    }
	  free_null_terminated_char_2D_array(lines);
	}
  }
  if (fd > 0) close(fd);
  free(buff);

  j = 0;
  if (c_row_index == 0)
    {
      color = malloc(3 * sizeof(*color));
      while (j < 3)
	{
	  color[j] = malloc(4 * sizeof(*color[j]));
	  j++;
	}
      color[0][0] = 0;
      color[0][1] = 0;
      color[0][2] = 0;
      color[0][3] = 0;

      color[1][0] = 1;
      color[1][1] = 1;
      color[1][2] = 1;
      color[1][3] = 1;

      color[2][0] = 99;
      color[2][1] = 0;
      color[2][2] = 0;
      color[2][3] = 0;
      return (color);
    }

  if (!flag)
    c_row_index++;
  colormap_tmp[c_row_index][0] = 99;

  color = malloc((c_row_index + 1) * sizeof(*color));
  while (colormap_tmp[j][0] != 99)
    {
      color[j] = malloc(4 * sizeof(*color[j]));
      color[j][0] = colormap_tmp[j][0];
      color[j][1] = colormap_tmp[j][1];
      color[j][2] = colormap_tmp[j][2];
      color[j][3] = colormap_tmp[j][3];
      j++;
    }
  color[j] = malloc(1 * sizeof(*color[j]));
  color[j][0] = 99;
  return (color);
}

int		count_files_presents_into_directory(char *path)
{
  DIR		*d = opendir(path);
  int		count = 0;
  char		*buf;
  struct dirent *p;
  struct stat	statbuf;

  if (d)
    {
      while ((p = readdir(d)))
	{
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
	    {
	      continue;
	    }
	  asprintf(&buf, "%s/%s", path, p->d_name);
	  if (!stat(buf, &statbuf))
	    {
	      if (!S_ISDIR(statbuf.st_mode))
		count++;
	    }
	  free(buf);
	}
      closedir(d);
    }
  return (count);
}

void		display_colormap(float **colormap, char *name)
{
  int		i = 0;

  INFO("\n\n Colormap Name = |%s|", name);
  while (colormap[i][0] != 99)
    {
      INFO("%f %f %f %f", colormap[i][0], colormap[i][1], colormap[i][2], colormap[i][3])
      i++;
    }
}

void		load_colormaps_from_directory(char *path, t_image_extract *image_args, short discrete)
{
	DIR *d = opendir(path);
	char *buf = NULL;
	struct dirent *p = NULL;
	struct stat statbuf;
	int i = 0;
	float **colormap_extracted = NULL;

	i = 0;
	// determine length of colormap array
	while (TRUE) {
		if (image_args->premapped_colormap[i] == NULL)
			break;
		i++;
	}

	if (d) {
		while ((p = readdir(d))) {
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;
			asprintf(&buf, "%s/%s", path, p->d_name);
			if (!stat(buf, &statbuf)) {
				if (!S_ISDIR(statbuf.st_mode)) {
					colormap_extracted =
							discrete ?
									get_colormap_from_lookup_file(buf) :
									get_colormap_from_file(buf);
					image_args->premapped_colormap[i] =
							malloc(
									(255 + 1)
											* sizeof(*image_args->premapped_colormap[i]));
					if (discrete) {
						alloc_and_init_colormap_space_from_lookup(
								image_args->premapped_colormap[i],
								colormap_extracted);

						// this is a hack to avoid a name clash
						int j=0;
						while (j<i) {
							if (strcmp(image_args->colormap_name[j], p->d_name) == 0) {
								asprintf(&image_args->colormap_name[i], "%s_lookup", strdup(p->d_name));
								break;
							} else {
								image_args->colormap_name[i] = strdup(p->d_name);
							}
							j++;
						}
					} else {
						alloc_and_init_colormap_space_from_src(
								image_args->premapped_colormap[i],
								colormap_extracted);
						image_args->colormap_name[i] = strdup(p->d_name);
					}
					i++;
				}
			}
			free(buf);
		}
		if (d>0) closedir(d);
	}
	image_args->premapped_colormap[i] = NULL;
	image_args->colormap_name[i] = NULL;
}

void		colormap_init(t_image_extract *image_args) {
  char * colormaps_path = strdup(CONCAT_APP_PATH("colormaps"));
  char * lookup_path = strdup(CONCAT_APP_PATH("lookup"));
  int number_of_colormaps = 0;
  int number_of_lookups = 0;

  number_of_colormaps = count_files_presents_into_directory(colormaps_path);
  INFO("Found %i colormaps in %s.\n", number_of_colormaps, colormaps_path);
  number_of_lookups = count_files_presents_into_directory(lookup_path);
  INFO("Found %i colormaps in %s.\n", number_of_lookups, lookup_path);

  image_args->premapped_colormap = malloc((number_of_colormaps + number_of_lookups + 1) * sizeof(*image_args->premapped_colormap));
  image_args->premapped_colormap[0] = NULL;
  image_args->colormap_name = malloc((number_of_colormaps + number_of_lookups + 1) * sizeof(*image_args->colormap_name));
  image_args->colormap_name[0] = NULL;

  if (number_of_colormaps) load_colormaps_from_directory(colormaps_path, image_args, FALSE);
  if (number_of_lookups) load_colormaps_from_directory(lookup_path, image_args, TRUE);

  free(colormaps_path);
  free(lookup_path);
}

void		get_percent(FILE *file, t_image_extract *a)
{
  char		buff[20];

  if (a->percent > 100)
    a->percent = 100;
  sprintf(buff, "%.2f", a->percent);
  if (file != NULL)
    {
      fwrite(buff, 1, strlen(buff), file);
      return;
    }
  DEBUG("%.2f%%", a->percent);
}

void		*percentage(void *args)
{
  t_image_args	*a;

  a = (t_image_args *)args;
  a->info->percent = 0;
  while (a->info->percent < 100)
    {
      pthread_mutex_lock(&(a->info->mut));
      if (a->info->percent < 100)
	pthread_cond_wait(&(a->info->cond), &(a->info->mut));
      a->info->percent = (100 / (float)a->info->total_slices_to_do) * a->info->slices_done;
      pthread_mutex_unlock(&(a->info->mut));
    }
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

t_image_args	*create_args_thread(t_tissue_stack *t, t_vol *vol, t_image_extract *image_general, FILE *sock)
{
  t_image_args   *args = malloc(sizeof(*args));
  // shared objects
  args->p = t->tp;
  args->file = sock;
  args->this = NULL; //  not needed for our purposes

  // copy and override pointers within struct.
  //note: we copy only what we need, the rest gets nulled to prevent pointers from being copied!

  // t_vol DEEP COPY
  args->volume = malloc(sizeof(*args->volume));
  memcpy(args->volume, vol, sizeof(*vol));

  if (vol->size != NULL) {
	  args->volume->size = malloc(sizeof(*args->volume->size) * args->volume->dim_nb);
	  memcpy(args->volume->size, vol->size, sizeof(*vol->size) * args->volume->dim_nb);
  }
  if (vol->slice_size != NULL) {
	  args->volume->slice_size = malloc(sizeof(*args->volume->slice_size) * args->volume->dim_nb);
	  memcpy(args->volume->slice_size, vol->slice_size, sizeof(*vol->slice_size) * args->volume->dim_nb);
  }
  if (vol->dim_offset != NULL) {
	  args->volume->dim_offset = malloc(sizeof(*args->volume->dim_offset) * args->volume->dim_nb);
	  memcpy(args->volume->dim_offset, vol->dim_offset, sizeof(*vol->dim_offset) * args->volume->dim_nb);
  }

  if (vol->dim_name_char != NULL) {
	  args->volume->dim_name_char = strdup(vol->dim_name_char);
  }

  if (vol->dim_name != NULL) {
	  args->volume->dim_name = malloc(sizeof(*args->volume->dim_name) * args->volume->dim_nb);
	  int i = 0;
	  while (i < args->volume->dim_nb) {
		  args->volume->dim_name[i] = strdup(vol->dim_name[i]);
		  i++;
	  }
  }
  args->volume->original_format = vol->original_format;

  if (vol->path != NULL) args->volume->path = strdup(vol->path);

  args->volume->dimensions = NULL;
  args->volume->next = NULL;
  args->volume->starts = NULL;
  args->volume->steps = NULL;

  // we leave the reference to the tissue stack instance, this is meant to be shared
  args->general_info = t;

  // t_image_extract DEEP COPY
  args->info = malloc(sizeof(*args->info));
  memcpy(args->info, image_general, sizeof(*image_general));

  if (image_general->dim_start_end != NULL) {
	  args->info->dim_start_end = malloc(sizeof(*image_general->dim_start_end) * args->volume->dim_nb);
	  int i = 0;
	  while (i < args->volume->dim_nb) {
		  args->info->dim_start_end[i] = malloc(sizeof(*image_general->dim_start_end[i]) * 2);
		  args->info->dim_start_end[i][0] =  image_general->dim_start_end[i][0];
		  args->info->dim_start_end[i][1] =  image_general->dim_start_end[i][1];
		  i++;
	  }
  }

  if (image_general->image_type != NULL) {
	  args->info->image_type = strdup(image_general->image_type);
  }
  if (image_general->root_path != NULL) {
	  args->info->root_path = strdup(image_general->root_path);
  }
  if (image_general->service != NULL) {
	  args->info->service = strdup(image_general->service);
  }
  if (image_general->request_id != NULL) {
	  args->info->request_id = strdup(image_general->request_id);
  }
  if (image_general->request_time != NULL) {
	  args->info->request_time = strdup(image_general->request_time);
  }

  return (args);
}

void		lunch_percent_display(t_tissue_stack *t, t_vol *vol, t_image_extract *image_general)
{
  t_image_args   *args;

  args = create_args_thread(t, vol, image_general, NULL);
  (*t->tp->add)(percentage, (void *)args, t->tp);
}

void		lunch_pct_and_add_task(t_tissue_stack *t, t_vol *vol, t_image_extract *image_general, int fd, char **commands)
{
  char		*command_line;
  char		*id_percent;

  pthread_mutex_lock(&image_general->percent_mut);
  if (image_general->percentage && image_general->id_percent == NULL)
    {
	  command_line = array_2D_to_array_1D(commands);
	  t->percent_init(get_nb_blocks_percent(image_general, vol), &id_percent, vol->path, "0", image_general->root_path, command_line, t);
	  if (write(fd, id_percent, 16) < 0)
		  ERROR("Open Error");
      image_general->id_percent = id_percent;
    }
  pthread_mutex_unlock(&image_general->percent_mut);
}

void		image_creation_lunch(t_tissue_stack *t, t_vol *vol, int step, t_image_extract *image_general, FILE *sock)
{
  t_image_args	*args;
  unsigned int	i = 0;
  unsigned int	j;
  unsigned int	nb_slices = 0;
  int		**dim_start_end;

  // infinite loop paranoia
  if (step <= 0) {
	  step = 1;
  }

  dim_start_end = image_general->dim_start_end;

  if (image_general->percentage)
    {
      // One thread only for each volume and all its dimentions
      args = create_args_thread(t, vol, image_general, sock);
      args->general_info = t;
      i = 0;
      args->dim_start_end = malloc(sizeof(*args->dim_start_end) * vol->dim_nb);
      while (i < vol->dim_nb) {
	args->dim_start_end[i] = malloc(sizeof(*args->dim_start_end[i]) * 2);
	args->dim_start_end[i][0] =  image_general->dim_start_end[i][0];
	args->dim_start_end[i][1] =  image_general->dim_start_end[i][1];
	i++;
      }
      (*t->tp->add)(get_all_slices_of_all_dimensions, (void *)args, t->tp);
      return;
    }

  i = 0;
  while (i < 3)
    {
      if (dim_start_end[i][0] != -1 && dim_start_end[i][1] != -1)
	{
	  j = 0;
	  nb_slices = ((dim_start_end[i][1] == 0 ? vol->size[i] : dim_start_end[i][1]) - dim_start_end[i][0]);
	  while (j < nb_slices)
	    {
	      args = create_args_thread(t, vol, image_general, sock);
	      args->general_info = t;
	      if ((dim_start_end[i][0] + step) <= dim_start_end[i][1])
		args->dim_start_end = generate_dims_start_end_thread(vol, i, dim_start_end[i][0], dim_start_end[i][0] + step);
	      else
		args->dim_start_end = generate_dims_start_end_thread(vol, i, dim_start_end[i][0], dim_start_end[i][1]);
#ifdef _NO_TILING_THREAD_
	      get_all_slices_of_all_dimensions(args);
#else
	      (*t->tp->add)(get_all_slices_of_all_dimensions, (void *)args, t->tp);
#endif
	      j += step;
	      dim_start_end[i][0] += step;
	    }
	}
      i++;
    }
}

int		check_colormap_name(char *str, t_image_extract *a)
{
  int		i;

  i = 0;
  while (a->colormap_name[i] != NULL)
    {
      if (strncmp(str, a->colormap_name[i], strlen(str)) == 0)
	{
	  if (strlen(str) == strlen(a->colormap_name[i]))
	    return (0);
	}
      i++;
    }
  return (1);
}

int		check_num_input(char **in, int start, int end)
{
  int		i;
  int		j;

  i = start;
  while (i < end)
    {
      if (strcmp(in[i], "-1") != 0)
	{
	  j = 0;
	  while (in[i][j] != '\0')
	    {
	      if (in[i][j] != '.')
		{
		  if (in[i][j] < '0' || in[i][j] > '9')
		    {
		      ERROR("Error invalid numerical input : %s", in[i]);
		      return (1);
		    }
		}
	      j++;
	    }
	}
      i++;
    }
  return (0);
}

int		check_range(int **d, t_vol *v)
{
  int		i;

  if (v == NULL)
    ERROR("Volume is NULL");
  i = 0;
  while (v && i < v->dim_nb)
    {
      if (d && d[i] && (d[i][0] != -1 || d[i][1] != -1))
	{
	  if (d[i][0] > v->size[i] || d[i][1] > v->size[i] || d[i][0] < 0 || d[i][1] < 0)
	    return (1);
	}
      i++;
    }
  return (0);
}

t_image_extract	*create_image_struct()
{
  t_image_extract	*image_args;

  image_args = malloc(sizeof(*image_args));
  image_args->total_slices_to_do = 0;
  image_args->slices_done = 0;
  image_args->step = 0;
  image_args->dim_start_end = NULL;
  image_args->root_path = NULL;
  image_args->service = NULL;
  image_args->image_type = NULL;
  image_args->request_id = NULL;
  image_args->request_time = NULL;
  pthread_mutex_init(&image_args->mut, NULL);
  pthread_cond_init(&image_args->cond, NULL);
  return (image_args);
}

void		*init(void *args)
{
  t_image_extract	*image_args;
  t_args_plug	*a;
  a = (t_args_plug *)args;
  prctl(PR_SET_NAME, "Image_plug");

  LOG_INIT(a);

  image_args = malloc(sizeof(*image_args));
  image_args->total_slices_to_do = 0;
  image_args->slices_done = 0;
  image_args->step = 0;
  image_args->dim_start_end = NULL;
  image_args->root_path = NULL;
  image_args->service = NULL;
  image_args->image_type = NULL;
  image_args->request_id = NULL;
  image_args->request_time = NULL;
  pthread_mutex_init(&image_args->mut, NULL);
  pthread_mutex_init(&image_args->percent_mut, NULL);
  pthread_cond_init(&image_args->cond, NULL);
  InitializeMagick("./");
  colormap_init(image_args);
  a->this->stock = (void*)image_args;

  INFO("Image Extract Plugin: Started");

  // free command line args
  a->destroy(a);

  return (NULL);
}

void			*start(void *args)
{
  t_image_extract	*image_args;
  t_image_extract	*image_args_tmp;
  t_args_plug		*a;
  int			step;
  char			*colormap;
  int			i = 0;
  FILE			*socketDescriptor;
  t_vol			*volume;

  a = (t_args_plug *)args;
  prctl(PR_SET_NAME, "TS_IMAGES");

  socketDescriptor = (FILE*)a->box;
  volume = load_volume(a, a->commands[0]);
  if (volume == NULL) {
    write_http_header(socketDescriptor, "500 Server Error", "png");
    fclose(socketDescriptor);
    return NULL;
  }

  image_args_tmp = (t_image_extract*)a->this->stock;
  image_args = create_image_struct();
  image_args->dim_nb = volume->dim_nb;
  image_args->premapped_colormap = image_args_tmp->premapped_colormap;
  image_args->colormap_name = image_args_tmp->colormap_name;
  image_args->percent_mut = image_args_tmp->percent_mut;

  if (strcmp(a->commands[1], "percent") == 0)
    {
      get_percent((FILE*)a->box, image_args);
      a->this->busy = 0;
      return (NULL);
    }
  if (check_num_input(a->commands, 1, 9))
    {
      a->this->busy = 0;
      return (NULL);
    }
  image_args->dim_start_end = generate_dims_start_end(volume,
						      atoi(a->commands[1]), atoi(a->commands[2]),
						      atoi(a->commands[3]), atoi(a->commands[4]),
						      atoi(a->commands[5]), atoi(a->commands[6]));
  if (check_range(image_args->dim_start_end, volume))
    {
      ERROR("Slice out of volume range");
      a->this->busy = 0;
      return (NULL);
    }

  image_args->square_size = -1;
  image_args->w_position = 0;
  image_args->h_position = 0;
  image_args->w_position_end = -1;
  image_args->h_position_end = -1;
  image_args->colormap_id = -1;
  image_args->scale = (float)atof(a->commands[7]);
  image_args->quality = atoi(a->commands[8]);
  image_args->service = a->commands[9];
  a->commands[10] = strupper(a->commands[10]);
  image_args->image_type = a->commands[10];
  image_args->percentage = 0;
  image_args->id_percent = NULL;

  if (strcmp(image_args->service, "tiles") == 0)
    {
      if (check_num_input(a->commands, 11, 14))
	{
	  a->this->busy = 0;
	  return (NULL);
	}
      image_args->square_size = atoi(a->commands[11]);
      image_args->h_position = atoi(a->commands[12]);
      image_args->w_position = atoi(a->commands[13]);
      image_args->start_h = atoi(a->commands[12]);
      image_args->start_w = atoi(a->commands[13]);
      if (a->commands[20] != NULL)
	{
	  image_args->root_path = strdup(a->commands[20]);
	  if (a->commands[21] != NULL && strcmp(a->commands[21], "@tasks@") == 0)
	    {
	      image_args->percentage = 1;
	      if (a->commands[22] != NULL)
		{
		  image_args->id_percent = strdup(a->commands[22]);
		  image_args->percent_fd = 1;
		}
	      else
		{
		  lunch_pct_and_add_task(a->general_info, volume, image_args, *((int*)a->box), a->commands);
		  return (NULL);
		}
	    }
	}
    }
  else if (strcmp(image_args->service, "images") == 0)
    {
      if (check_num_input(a->commands, 11, 15))
	{
	  a->this->busy = 0;
	  return (NULL);
	}
      image_args->h_position = atoi(a->commands[11]);
      image_args->w_position = atoi(a->commands[12]);
      image_args->start_h = atoi(a->commands[11]);
      image_args->start_w = atoi(a->commands[12]);
      image_args->h_position_end = atoi(a->commands[13]);
      image_args->w_position_end = atoi(a->commands[14]);
      if (a->commands[21] != NULL)
	image_args->root_path = strdup(a->commands[21]);
    }
  else
    {
      if (strcmp(image_args->service, "full") != 0)
	{
	  ERROR("Undefined kind. Please choose 'tiles' or 'images'");
	  a->this->busy = 0;
	  return (NULL);
	}
      else
	{
	  if (a->commands[17] != NULL)
	    {
	      image_args->root_path = strdup(a->commands[17]);
	      if (a->commands[18] != NULL && strcmp(a->commands[18], "@tasks@") == 0)
		{
		  prctl(PR_SET_NAME, "TS_TILING");
		  image_args->percentage = 1;
		  if (a->commands[19] != NULL)
		    {
		      image_args->id_percent = strdup(a->commands[19]);
		      image_args->percent_fd = 1;
		    }
		  else
		    {
		      lunch_pct_and_add_task(a->general_info, volume, image_args, *((int*)a->box), a->commands);
		      return (NULL);
		    }
		}
	    }
	}
    }
  image_args->total_slices_to_do = get_total_slices_to_do(volume, image_args->dim_start_end);

  i = 0;
  colormap = (strcmp(image_args->service, "tiles") == 0 ? strdup(a->commands[14]) : (strcmp(image_args->service, "full") == 0 ? strdup(a->commands[11]) : strdup(a->commands[15])));
  if (check_colormap_name(colormap, image_args))
    WARNING("Warning : colormap '%s' does not exist", colormap);
  while (image_args->colormap_name[i] != NULL)
    {
      if (strncmp(colormap, image_args->colormap_name[i], strlen(colormap)) == 0 && strlen(image_args->colormap_name[i]) == strlen(colormap))
	break;
      i++;
    }
  if (image_args->colormap_name[i] != NULL)
    image_args->colormap_id = i;
  else
    image_args->colormap_id = -1;

  image_args->contrast_min = (strcmp(image_args->service, "tiles") == 0 ? atoi(a->commands[15]) : (strcmp(image_args->service, "full") == 0 ? atoi(a->commands[12]) : atoi(a->commands[16])));
  image_args->contrast_max = (strcmp(image_args->service, "tiles") == 0 ? atoi(a->commands[16]) : (strcmp(image_args->service, "full") == 0 ? atoi(a->commands[13]) : atoi(a->commands[17])));

  if (image_args->contrast_max != 0)
    image_args->contrast = 1;
  else
    image_args->contrast = 0;

  step = (strcmp(image_args->service, "tiles") == 0 ? atoi(a->commands[17]) : (strcmp(image_args->service, "full") == 0 ? atoi(a->commands[14]) : atoi(a->commands[18])));

  image_args->request_id = (strcmp(image_args->service, "tiles") == 0 ? strdup(a->commands[18]) : (strcmp(image_args->service, "full") == 0 ? strdup(a->commands[15]) : strdup(a->commands[19])));
  image_args->request_time = (strcmp(image_args->service, "tiles") == 0 ? strdup(a->commands[19]) : (strcmp(image_args->service, "full") == 0 ? strdup(a->commands[16]) : strdup(a->commands[20])));

  image_creation_lunch(a->general_info,volume, step, image_args, socketDescriptor);

  a->destroy(a);

  return (NULL);
}

void		*unload(void *args)
{
  t_image_extract	*image_args = NULL;
  t_args_plug	*a = NULL;

  a = (t_args_plug *)args;

  image_args = (t_image_extract *)a->this->stock;

  free_image_extract(image_args);

  if (a != NULL) {
	  free(a->name);
	  free(a->path);
	  free(a);
	  a = NULL;
  }

  DestroyMagick();

  return (NULL);
}

void			free_image_extract(t_image_extract * extract) {
	if (extract == NULL) {
		return;
	}

	if (extract->dim_start_end != NULL) {
		int i=0;
		while (i<extract->dim_nb) {
			if (extract->dim_start_end[i] != NULL) free(extract->dim_start_end[i]);
			i++;
		}
		free(extract->dim_start_end);
	}

	if (extract->image_type != NULL) free(extract->image_type);
	if (extract->root_path != NULL)  free(extract->root_path);
	if (extract->service != NULL)    free(extract->service);
	if (extract->request_id != NULL)    free(extract->request_id);
	if (extract->request_time != NULL)    free(extract->request_time);

	free(extract);
	extract = NULL;
}

void			free_image_args(t_image_args * args) {
	if (args == NULL) {
		return;
	}

	if (args->this != NULL) free(args->this);

	free_image_extract(args->info);

	if (args->dim_start_end != NULL) {
		int i=0;
		while (i<args->volume->dim_nb) {
			if (args->dim_start_end[i] != NULL) free(args->dim_start_end[i]);
			i++;
		}
		free(args->dim_start_end);
	}

	if (args->volume != NULL) {
		if (args->volume->size != NULL) free(args->volume->size);
		if (args->volume->path != NULL) free(args->volume->path);

		if (args->volume->dim_name != NULL) {
			int i=0;
			while (i<args->volume->dim_nb) {
				if (args->volume->dim_name[i] != NULL) free(args->volume->dim_name[i]);
				i++;
			}
			free(args->volume->dim_name);
		}
		if (args->volume->starts != NULL) free(args->volume->starts);
		if (args->volume->steps != NULL) free(args->volume->steps);
		if (args->volume->dim_name_char != NULL) free(args->volume->dim_name_char);
		if (args->volume->dim_offset != NULL) free(args->volume->dim_offset);
		if (args->volume->slice_size != NULL) free(args->volume->slice_size);

		free(args->volume);
	}

	// set shared pointer to requests to null
	args->general_info = NULL;

	free(args);
}
