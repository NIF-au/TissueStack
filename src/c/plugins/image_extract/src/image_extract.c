#include "image_extract.h"

float		colormapa[4][25][4] = {{{0, 0, 0, 0},
					{0.05, 0.46667, 0, 0.05333},
					{0.1, 0.5333, 0, 0.6},
					{0.15, 0, 0, 0.6667},
					{0.2, 0, 0, 0.8667},
					{0.25, 0, 0.4667, 0.8667},
					{0.3, 0, 0.6, 0.8667},
					{0.35, 0, 0.6667, 0.6667},
					{0.4, 0, 0.6667, 0.5333},
					{0.45, 0, 0.6, 0},
					{0.5, 0, 0.7333, 0},
					{0.55, 0, 0.8667, 0},
					{0.6, 0, 1, 0},
					{0.65, 0.7333, 1, 0},
					{0.7, 0.9333, 0.9333, 0},
					{0.75, 1, 0.8, 0},
					{0.8, 1, 0.6, 0},
					{0.85, 1, 0, 0},
					{0.9, 0.8667, 0, 0},
					{0.95, 0.8, 0, 0},
					{1, 0.8, 0.8, 0.8},
					{99, 0, 0, 0}},
				       {{0, 0, 0, 0},
					{0.25, 0.5, 0, 0},
					{0.5, 1, 0.5, 0},
					{0.75, 1, 1, 0.5},
					{1, 1, 1, 1},
					{99, 0, 0, 0}},
				       {{0, 0, 0, 0},
					{1, 1, 1, 1},
					{99, 0, 0, 0}},
				       {{0, 0, 0, 0},
					{1, 1, 1, 1},
					{99, 0, 0, 0}}};

char		*colormapa_name[5] = {"spectral", "hot", "gray", "grey", NULL};

int		get_size_colormap(float **colormap)
{
  int		i;

  i = 0;
  while (colormap[i][0] != 99)
    i++;
  return (i);
}

void		alloc_and_init_colormap_space(float **new_colormap, int index)
{
  int		i;
  int		j;
  float		start_red;
  float		end_red;
  float		start_green;
  float		end_green;
  float		start_blue;
  float		end_blue;
  float		start_range;
  float		end_range;
  float		red_delta;
  float		green_delta;
  float		blue_delta;
  float		delta;
  int		end_loop;

  i = 0;
  j = 1;
  while (colormapa[index][j][0] < 99)
    {
      start_range	= floor(colormapa[index][j - 1][0] * 255);
      end_range		= floor(colormapa[index][j][0] * 255);
      delta		= end_range - start_range;

      start_red		= colormapa[index][j - 1][1];
      end_red		= colormapa[index][j][1];

      start_green	= colormapa[index][j - 1][2];
      end_green		= colormapa[index][j][2];

      start_blue	= colormapa[index][j - 1][3];
      end_blue		= colormapa[index][j][3];

      red_delta = round((end_red - start_red) / delta);
      green_delta = round((end_green - start_green) / delta);
      blue_delta = round((end_blue - start_blue) / delta);

      end_loop = i + delta;
      while (i < end_loop)
	{
	  new_colormap[i] = malloc(3 * sizeof(*new_colormap[i]));

	  new_colormap[i][0] = round((i * start_red) + ((end_red - i) * red_delta));
	  new_colormap[i][1] = round((i * start_green) + ((end_green - i) * green_delta));
	  new_colormap[i][2] = round((i * start_blue) + ((end_blue - i) * blue_delta));
	  i++;
	}
      j++;
    }
}

void		alloc_and_init_colormap_space_from_src(float **new_colormap, float **source)
{
  int		i;
  int		j;
  float		start_red;
  float		end_red;
  float		start_green;
  float		end_green;
  float		start_blue;
  float		end_blue;
  float		start_range;
  float		end_range;
  float		red_delta;
  float		green_delta;
  float		blue_delta;
  float		delta;
  int		end_loop;

  i = 0;
  j = 1;
  while (source[j][0] < 99)
    {
      start_range	= floor(source[j - 1][0] * 255);
      end_range		= floor(source[j][0] * 255);
      delta		= end_range - start_range;

      start_red		= source[j - 1][1];
      end_red		= source[j][1];

      start_green	= source[j - 1][2];
      end_green		= source[j][2];

      start_blue	= source[j - 1][3];
      end_blue		= source[j][3];

      red_delta = round((end_red - start_red) / delta);
      green_delta = round((end_green - start_green) / delta);
      blue_delta = round((end_blue - start_blue) / delta);

      end_loop = i + delta;
      while (i < end_loop)
	{
	  new_colormap[i] = malloc(3 * sizeof(*new_colormap[i]));

	  new_colormap[i][0] = round((i * start_red) + ((end_red - i) * red_delta));
	  new_colormap[i][1] = round((i * start_green) + ((end_green - i) * green_delta));
	  new_colormap[i][2] = round((i * start_blue) + ((end_blue - i) * blue_delta));
	  i++;
	}
      j++;
    }
}

void		colormap_init(t_image_extract *image_args)
{
  int		i;
  int		clormp_nb;

  clormp_nb = 2;
  image_args->premapped_colormap = malloc((clormp_nb + 1) * sizeof(*image_args->premapped_colormap));
  i = 0;
  while (i < clormp_nb)
    {
      image_args->premapped_colormap[i] = malloc((255 + 1) * sizeof(*image_args->premapped_colormap[i]));
      alloc_and_init_colormap_space(image_args->premapped_colormap[i], i);
      i++;
    }
  image_args->colormap_name = colormapa_name;
  image_args->premapped_colormap[clormp_nb] = NULL;
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

void		image_creation_lunch(t_tissue_stack *t, t_vol *vol, int step, t_image_extract *image_general, FILE *sock)
{
  t_image_args	*args;
  unsigned int	i;
  unsigned int	j;
  unsigned int	nb_slices = 0;
  int		**dim_start_end;
  char		*id_percent = NULL;
  char		*zoom_f;

  i = 0;

  // infinite loop paranoia
  if (step <= 0) {
	  step = 1;
  }

  dim_start_end = image_general->dim_start_end;

  if (image_general->percentage && image_general->id_percent == NULL)
    {
      asprintf(&zoom_f, "%f", image_general->scale);
      t->percent_init(get_nb_blocks_percent(image_general, vol), &id_percent, vol->path, "0", image_general->root_path, zoom_f, t);
      if (write(image_general->percent_fd, id_percent, 10) < 0)
	ERROR("Open Error");
      image_general->id_percent = id_percent;
      free(zoom_f);
    }

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
	      (*t->tp->add)(get_all_slices_of_all_dimensions, (void *)args, t->tp);
	      j += step;
	      dim_start_end[i][0] += step;
	    }
	}
      i++;
    }
}

int		check_colormap_name(char *str)
{
  int		i;

  i = 0;
  while (colormapa_name[i] != NULL)
    {
      if (strcmp(str, colormapa_name[i]) == 0)
	return (0);
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
  if (v == NULL) ERROR("Volume is NULL");

  int		i;

  i = 0;
  while (i < v->dim_nb)
    {
      if (d[i][0] != -1 || d[i][1] != -1)
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
  InitializeMagick("./");
  colormap_init(image_args);
  a->this->stock = (void*)image_args;

  LOG_INIT(a);

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
	  if (a->commands[21] != NULL && strcmp(a->commands[21], "@tiling@") == 0)
	    {
	      image_args->percentage = 1;
	      if (a->commands[22] != NULL)
		{
		  image_args->id_percent = strdup(a->commands[22]);
		  image_args->percent_fd = 1;
		}
	      else
		image_args->percent_fd = *((int*)a->box);

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
	      if (a->commands[18] != NULL && strcmp(a->commands[18], "@tiling@") == 0)
		{
		  image_args->percentage = 1;
		  if (a->commands[19] != NULL)
		    {
		      image_args->id_percent = strdup(a->commands[19]);
		      image_args->percent_fd = 1;
		    }
		  else
		    image_args->percent_fd = *((int*)a->box);
		}
	    }
	}
    }
  image_args->total_slices_to_do = get_total_slices_to_do(volume, image_args->dim_start_end);

  i = 0;
  colormap = (strcmp(image_args->service, "tiles") == 0 ? strdup(a->commands[14]) : (strcmp(image_args->service, "full") == 0 ? strdup(a->commands[11]) : strdup(a->commands[15])));
  if (check_colormap_name(colormap))
    ERROR("Warning : colormap '%s' does not exist", colormap);
  while (colormapa_name[i] != NULL)
    {
      if (strcmp(colormap, colormapa_name[i]) == 0)
	break;
      i++;
    }
  if (colormapa_name[i] != NULL && strcmp(colormapa_name[i], "gray") != 0 && strcmp(colormapa_name[i], "grey") != 0)
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
  }

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
