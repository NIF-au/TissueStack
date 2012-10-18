#include "core.h"
#include <nifti1_io.h>

int		is_num(char *str)
{
  int		i = 0;

  while (str[i] != '\0')
    {
      if ((str[i] > '9' || str[i] < '0') && str[i] != '.')
	return (0);
      i++;
    }
  return (1);
}

int		percent_word_count(char *buff, char c)
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

int		percent_letter_count(char *buff, int position, char c)
{
  int		i;

  i = 0;
  while (buff[position + i] != c && buff[position + i] != '\0')
    i++;
  return (i);
}

char		**percent_str_to_wordtab(char *buff, char c)
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
  wordCount = percent_word_count(buff, c);
  if (wordCount == 0) {
	  return NULL;
  }

  dest = malloc((wordCount + 1) * sizeof(*dest));

  while (buff[i] != '\0')
    {
      if (buff[i] != c && buff[i] != '\0')
	{
	  dest[j] = str_n_cpy(buff, i, percent_letter_count(buff, i, c));
	  j++;
	  i += percent_letter_count(buff, i, c);
	}
      if (buff[i] != '\0')
	i++;
    }

  // terminate 2D array with NULL
  dest[j] = NULL;

  return (dest);
}

void		percent_init_direct(int total_blocks, char **id, char *filename, char *kind, char *path, char *zoom_factor, t_tissue_stack *t)
{
  char		*str_log;
  struct timeval tv;
  char		*complete_path;
  int		len_path;
  FILE		*f;

  gettimeofday(&tv,NULL);
  *id = malloc(17 * sizeof(**id));
  memset(*id, '0', 17);
  snprintf(*id, 17, "%i%06i", (int)tv.tv_sec, (int)tv.tv_usec);
  if (t->percent->path)
    {
      len_path = (strlen(*id) + strlen(t->percent->path));
      complete_path = malloc((len_path + 1) * sizeof(*complete_path));
      complete_path = strcpy(complete_path, t->percent->path);
      complete_path = strcat(complete_path, *id);
      f = fopen(complete_path, "w+");
      if (zoom_factor != NULL)
	fprintf(f, "0\n%i\n%i\n%s\n%s\n%s\n%s\n", 0, total_blocks, filename, kind, path, zoom_factor);
      else
	fprintf(f, "0\n%i\n%i\n%s\n%s\n%s\n", 0, total_blocks, filename, kind, path);
      fclose(f);
      free(complete_path);
      str_log = *id;
      INFO("Percentage: %s Initialized", str_log);
    }
}

char		**read_from_file_by_id(char *id, FILE **f, t_tissue_stack *t)
{
  char		*complete_path;
  struct stat	info;
  char		**result;
  char		buff[4096];

  *f = NULL;
  if (t->percent->path)
    {
      complete_path = malloc((strlen(id) + strlen(t->percent->path) + 1) * sizeof(*complete_path));
      complete_path = strcpy(complete_path, t->percent->path);
      complete_path = strcat(complete_path, id);
      if (!stat(complete_path, &info))
	{
	  *f = fopen(complete_path, "r+");
	  if (fread(buff, 1, 4096, *f) > 0)
	    result = percent_str_to_wordtab(buff, '\n');
	  fseek(*f, 0, SEEK_SET);
	  free(complete_path);
	  return (result);
	}
    }
  return (NULL);
}

void		percent_add_direct(int blocks, char *id, t_tissue_stack *t)
{
  char		**result;
  float		percent_tmp = 0;
  float		blocks_done;
  FILE		*f;


  if (id == NULL || t->percent == NULL)
    return;
  pthread_mutex_lock(&t->percent->mutex);
  if ((result = read_from_file_by_id(id, &f, t)) != NULL)
    {
      if (strcmp(result[0], "100") != 0)
	{
	  blocks_done = atof(result[1]);
	  blocks_done += blocks;
	  percent_tmp = (float)((float)((float)blocks_done / (float)atof(result[2])) * 100.0);

	  fprintf(f, "%f\n%f\n%s\n%s\n%s\n%s\n%s\n", percent_tmp, blocks_done, result[2], result[3], result[4], result[5], result[6]);
	  fclose(f);
	}
    }
  pthread_mutex_unlock(&t->percent->mutex);
  if (percent_tmp >= 100)
    INFO("Percentage: %s ==> 100%%", id);
}

void		percent_get_direct(char **buff, char *id, t_tissue_stack *t)
{
  FILE		*f;
  char		**result;

  if (t->percent == NULL)
    return;
  if ((result = read_from_file_by_id(id, &f, t)) != NULL)
    {
      *buff = strdup(result[0]);
      fclose(f);
    }
  else
    asprintf(buff, "ERROR");
}

int		get_dim_size(t_vol *volume, char c)
{
  int		i = 0;

  while (i < volume->dim_nb)
    {
      if (volume->dim_name_char[i] == c)
	return (volume->size[i]);
      i++;
    }
  return (0);
}

void		get_width_height(int *height, int *width, int current_dimension,
				 t_vol *volume)
{
  if (volume->dim_name_char[current_dimension] == 'x')
    {
      *height = get_dim_size(volume, 'z'); //volume->size[Y];
      *width = get_dim_size(volume, 'y'); //volume->size[Z];
    }
  else if (volume->dim_name_char[current_dimension] == 'y')
    {
      *height = get_dim_size(volume, 'z');//volume->size[X];
      *width = get_dim_size(volume, 'x');//volume->size[Z];
    }
  else
    {
      *height = get_dim_size(volume, 'y');//volume->size[X];
      *width = get_dim_size(volume, 'x');//volume->size[Y];
    }
}

void		percent_resume_direct(char *id, t_tissue_stack *t)
{
  char		**result;
  t_vol		*vol;
  int		blocks_done;
  int		i = 0;
  int		tiles_nb[3];
  char		*comm;
  int		dim_s_e[3][2];
  FILE		*f;
  int		width;
  int		height;
  int		w_tiles;
  int		h_tiles;
  float		scale;
  int		tiles_per_slice[3];
  t_pause_cancel_queue *tmp;
  t_pause_cancel_queue *tmp2;

  if ((result = read_from_file_by_id(id, &f, t)) != NULL)
    {
      DEBUG("hellow 1");
      if (strcmp(result[0], "100") != 0)
	{
	  DEBUG("hellow 2");
	  if ((vol = t->get_volume(result[3], t)) != NULL)
	    {
	      DEBUG("hellow 3");
	      if ((tmp = t->percent->cancel_first) != NULL)
		{
		  DEBUG("hellow 4");
		  if (strcmp(id, tmp->id) == 0)
		    {
		      t->percent->cancel_first = tmp->next;
		      free(tmp->id);
		      free(tmp);
		    }
		  else
		    {
		      while (tmp)
			{
			  if (tmp->next && strcmp(id, tmp->next->id) == 0)
			    break;
			  tmp = tmp->next;
			}
		      if (tmp && tmp->next)
			{
			  tmp2 = tmp->next->next;
			  free(tmp->next->id);
			  free(tmp->next);
			  tmp->next = tmp2;
			}
		    }
		}
	      DEBUG("hellow 5")
	      blocks_done = atoi(result[1]);
	      if (result[4][0] == '0')
		{
		  scale = atof(result[6]);
		  i = 0;
		  while (i < vol->dim_nb)
		    {
		      get_width_height(&height, &width, i, vol);
		      h_tiles = (height * scale) / 256;
		      w_tiles = (width * scale) / 256;
		      if (height % 256 != 0)
			h_tiles++;
		      if (width % 256 != 0)
			w_tiles++;
		      tiles_per_slice[i] = h_tiles * w_tiles;
		      tiles_nb[i] = (h_tiles * w_tiles) * vol->size[i];
		      i++;
		    }
		  i = 0;
		  while (i < vol->dim_nb)
		    {
		      if (tiles_nb[i] > blocks_done && blocks_done > 0)
			{
			  dim_s_e[i][0] = blocks_done / tiles_per_slice[i];
			  dim_s_e[i][1] = 0;
			}
		      else if (tiles_nb[i] < blocks_done)
			{
			  dim_s_e[i][0] = -1;
			  dim_s_e[i][1] = -1;
			}
		      else if (blocks_done <= 0)
			{
			  dim_s_e[i][0] = 0;
			  dim_s_e[i][1] = 0;
			}
		      blocks_done -= tiles_nb[i];
		      i++;
		    }
		  if ("full")
		    {
		      asprintf(&comm, "start image %s %i %i %i %i %i %i %.4f %i tiles JPEG 256 -1 -1 grey 0 0 10000 0 0 %s @tiling@ %s",
			       result[3],
			       dim_s_e[0][0], dim_s_e[0][1],
			       dim_s_e[1][0], dim_s_e[1][1],
			       dim_s_e[2][0], dim_s_e[2][1],
			       atof(result[6]),
			       1,
			       result[5],
			       id);
		    }
		  else if ("preview")
		    {
		      asprintf(&comm, "start image %s %i %i %i %i %i %i %.4f %i full JPEG grey 0 0 10000 0 0 %s @tiling@ %s",
			       result[3],
			       dim_s_e[0][0], dim_s_e[0][1],
			       dim_s_e[1][0], dim_s_e[1][1],
			       dim_s_e[2][0], dim_s_e[2][1],
			       atof(result[6]),
			       6,
			       result[5],
			       id);
		    }
		  DEBUG("%s\n", comm);
		  t->plug_actions(t, comm, NULL);
		}
	      else if (result[4][0] == '1')
		{
		  int		slice = 0;
		  int		dimension = 0;
		  t_vol		*vol;
		  int		blocks_done;
		  int		blocks_calculed = 0;
		  int		i = 0;

		  if ((vol = get_volume(result[3], t)) == NULL)
		    {
		      add_volume(result[3], t);
		      vol = get_volume(result[3], t);
		    }
		  blocks_done = atoi(result[1]);
		  while (i < 3)
		    {
		      if (blocks_calculed + vol->size[i] < blocks_done)
			blocks_calculed += vol->size[i];
		      else
			{
			  slice = blocks_done - blocks_calculed;
			  dimension = i;
			  break;
			}
		      i++;
		    }
		  asprintf(&comm, "start minc_converter %s %s %i %i %s",
			   result[3], result[5], dimension, (slice - 2), id);
		  DEBUG("%s", comm);
		  t->plug_actions(t, comm, NULL);
		}
	    }
	  else if (result[4][0] == '2')
	    {
	      int		slice = 0;
	      int		dimension = 0;
	      int		blocks_done;
	      int		blocks_calculed = 0;
	      int		i = 0;
	      int		sizes[3];
	      nifti_image	*nim;

	      DEBUG("hellow 4");
	      if ((nim = nifti_image_read(result[3], 0)) == NULL)
		{
		  ERROR("Error Nifti read");
		  return;
		}

	      sizes[0] = nim->dim[1];
	      sizes[1] = nim->dim[2];
	      sizes[2] = nim->dim[3];

	      blocks_done = atoi(result[1]);
	      while (i < 3)
		{
		  if (blocks_calculed + sizes[i] < blocks_done)
		    blocks_calculed += sizes[i];
		  else
		    {
		      slice = blocks_done - blocks_calculed;
		      dimension = i;
		      break;
		    }
		  i++;
		}
	      asprintf(&comm, "start nifti_converter %s %s %i %i %s",
		       result[3], result[5], dimension, (slice - 2), id);
	      DEBUG("%s", comm);
	      t->plug_actions(t, comm, NULL);
	    }
	}
    }
  fclose(f);
}

void		percent_pause_direct(char *id, t_tissue_stack *t)
{
  t_pause_cancel_queue	*tmp;

  if (t && id)
    {
      if ((tmp = t->percent->cancel_first) != NULL)
	{
	  while (tmp->next)
	    tmp = tmp->next;
	  tmp->next = malloc(sizeof(*tmp->next));
	  tmp = tmp->next;
	  tmp->id = strdup(id);
	  tmp->next = NULL;
	}
      else
	{
	  tmp = malloc(sizeof(*tmp));
	  tmp->next = NULL;
	  tmp->id = strdup(id);
	  t->percent->cancel_first = tmp;
	}
    }
}

void		clean_pause_queue(char *id, t_tissue_stack *t)
{
  t_pause_cancel_queue	*tmp;
  t_pause_cancel_queue	*save;

  if (id && t)
    {
      tmp = t->percent->cancel_first;
      if (strcmp(tmp->id, id) == 0)
	{
	  t->percent->cancel_first = tmp->next;
	  free(tmp->id);
	  free(tmp);
	  return;
	}
      while (tmp)
	{
	  if (tmp->next && strcmp(tmp->next->id, id) == 0)
	    {
	      save = tmp->next;
	      tmp->next = tmp->next->next;
	      free(save->id);
	      free(save);
	      return;
	    }
	  tmp = tmp->next;
	}
    }
}

int		is_percent_paused_cancel(char *id, t_tissue_stack *t)
{
  t_pause_cancel_queue	*tmp;

  if (t && id)
    {
      tmp = t->percent->cancel_first;
      while (tmp)
	{
	  if (strcmp(id, tmp->id) == 0)
	    return (1);
	  tmp = tmp->next;
	}
    }
  return (0);
}

/*
void		percent_destroy(char **commands, void *box, t_tissue_stack *t)
{
  t_percent_elem *tmp;
  t_percent_elem *sav;
  char		*str_log;

  if (t->percent == NULL || (tmp = t->percent->first_percent) == NULL ||
      commands == NULL || commands[1] == NULL || commands[2] == NULL)
    return;
  if (strcmp(t->percent->first_percent->id, commands[1]) == 0)
    ;
  else
    {
      while (tmp != NULL)
	{
	  if (tmp->next != NULL)
	    {
	      if (strcmp(tmp->next->id, commands[1]) == 0)
		break;
	    }
	  tmp = tmp->next;
	}
    }
  if (tmp == NULL)
    return;
  str_log = tmp->next->id;
  INFO("Percentage: %s Destroyed", str_log);
  sav = tmp->next->next;
  free(tmp->next->id);
  free(tmp->next->time);
  free(tmp->next);
  tmp->next = sav;
}
*/

void		init_percent_time(t_tissue_stack *t, char *path)
{
  t_prcnt_t	*p;
  t_string_buffer * actualPath;

  p = malloc(sizeof(*p));
  t->percent = p;
  if (path != NULL)
    {
      pthread_mutex_init(&t->percent->mutex, NULL);
      t->percent->path = strdup(path);
      actualPath = createDirectory(path, 0766);
      if (actualPath == NULL) {
	ERROR("Couldn't create %s", path);
      }
      else
	free_t_string_buffer(actualPath);
    }
  else
    t->percent->path = NULL;
  t->percent->cancel_first = NULL;
  INFO("Percent initialized");
}
