#include "percent_and_time.h"

void		percent_time_write_plug(char *str, void *box)
{
  DEBUG("percent = %s", str);
  if (str && box)
    write(*((int *)box), str, strlen(str));
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
	  free(complete_path);
	  return (result);
	}
    }
  return (NULL);
}


void		percent_get(char *id, void *box, t_tissue_stack *t)
{
  char		pc[4096];
  char		**result;
  FILE		*f;

  if (t->percent == NULL)
    return;
  if ((result = read_from_file_by_id(id, &f, t)) != NULL)
    {
      sprintf(pc, "%s|%s", result[3], result[0]);
      fclose(f);
    }
  else
    sprintf(pc, "NULL");
  percent_time_write_plug(pc, box);
}

void		percent_cancel(char *id, t_tissue_stack *t)
{
  t_cancel_queue	*tmp;

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

void		percent_resume(char *id, t_tissue_stack *t)
{
  char		**result;
  t_vol		*vol;
  int		blocks_done;
  int		i = 0;
  int		count = 0;
  int		dim_size = 0;
  char		*dest;
  int		dim_s_e[3][2];
  FILE		*f;

  if ((result = read_from_file_by_id(id, &f, t)) != NULL)
    {
      if (strcmp(result[0], "100") != 0)
	{
	  if ((vol = t->get_volume(result[3], t)) != NULL)
	    {
	      blocks_done = atoi(result[2]) - 1;
	      if (result[4][0] == '0')
		{
		  while (i < vol->dim_nb)
		    {
		      dim_size = vol->size[i];
		      if ((count + dim_size) > blocks_done)
			{
			  dim_s_e[i][0] = blocks_done;
			  dim_s_e[i][1] = 0;
			}
		      if (count > blocks_done)
			{
			  dim_s_e[i][0] = 0;
			  dim_s_e[i][1] = 0;
			}
		      if ((count + dim_size) < blocks_done)
			{
			  dim_s_e[i][0] = -1;
			  dim_s_e[i][1] = -1;
			}
		      if ("full")
			asprintf(&dest, "start image %i %i %i %i %i %i %.4f %i tiles JPEG 256 -1 -1 grey 0 0 1000 0 0 %s @tiling@ %s",
				 dim_s_e[0][0], dim_s_e[0][1],
				 dim_s_e[1][0], dim_s_e[1][1],
				 dim_s_e[2][0], dim_s_e[2][1],
				 atof(result[5]),
				 1,
				 result[6],
				 id);
		      else if ("preview")
			asprintf(&dest, "start image %i %i %i %i %i %i %.4f %i full JPEG grey 0 0 1000 0 0 %s @tiling@ %s",
				 dim_s_e[0][0], dim_s_e[0][1],
				 dim_s_e[1][0], dim_s_e[1][1],
				 dim_s_e[2][0], dim_s_e[2][1],
				 atof(result[5]),
				 6,
				 result[6],
				 id);
		      t->plug_actions(t, dest, NULL);
		      break;
		      i++;
		    }
		}
	      /*
	      else if (result[4][0] == '1')
		;// minc conv
	      else if (result[4][0] == '2')
		;// nifti conv
	      */
	    }
	}
    }
  fclose(f);
}

void		*init(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  LOG_INIT(a);
  INFO("Initialized");
  return (NULL);
}

void		*start(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;

  DEBUG("Started getting: %s", a->commands[0]);
  if (strcmp(a->commands[0], "get") == 0)
    percent_get(a->commands[1], a->box, a->general_info);
  else if (strcmp(a->commands[0], "cancel") == 0)
    percent_cancel(a->commands[1], a->general_info);
  else if (strcmp(a->commands[0], "pause") == 0)
    percent_cancel(a->commands[1], a->general_info);
  else if (strcmp(a->commands[0], "pause") == 0)
    percent_resume(a->commands[1], a->general_info);
  return (NULL);
}

void		*unload(void *args)
{
  INFO("Unloaded");
  return (NULL);
}
