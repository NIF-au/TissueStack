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
#include "percent_and_time.h"

void		percent_time_write_plug(char *str, void *box)
{
  DEBUG("percent = %s", str);
  if (str && box)
    write(*((int *)box), str, strlen(str));
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
  while (buff && buff[position + i] != c && buff[position + i] != '\0')
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

char **read_from_file_by_id(char *id, t_tissue_stack *t) {

	FILE * f = NULL;
	char **result = NULL;
	char buff[4096];

	f = open_file_by_id(id, t);

	if (f == NULL) 	return NULL;

	memset(buff, '\0', 4095);
	if (fread(buff, 1, 4096, f) > 0)
		result = percent_str_to_wordtab(buff, '\n');
	fseek(f, 0, SEEK_SET);

	fclose(f);
	f = NULL;

	return (result);
}

FILE * open_file_by_id(char *id, t_tissue_stack *t) {
	if (id == NULL || t == NULL || t->percent == NULL || t->percent->path == NULL)
		return NULL;

	char	* complete_path = NULL;
    FILE * fh = NULL;

    asprintf(&complete_path, "%s/%s", t->percent->path, id);

    fh = fopen(complete_path, "r+");
    free(complete_path);

    return fh;
}

void		percent_get(char *id, void *box, t_tissue_stack *t)
{
  char		pc[4096];
  char		**result = NULL;

  if (t->percent == NULL)
    return;
  if ((result = read_from_file_by_id(id, t)) != NULL)
    sprintf(pc, "%s|%s", result[3], result[0]);
  else
    sprintf(pc, "NULL");

  if (result != NULL) free_null_terminated_char_2D_array(result);

  percent_time_write_plug(pc, box);
}

void		percent_cancel(char *id, void *box, t_tissue_stack *t)
{
  percent_get(id, box, t);
  t->percent_pause(id, t);
  sleep(1);
  t->percent_cancel(id, t);
}

void		percent_pause(char *id, void *box, t_tissue_stack *t)
{
  t->percent_pause(id, t);
  percent_get(id, box, t);
}

void		percent_resume(char *id, void *box, t_tissue_stack *t)
{
  t->percent_resume(id, t);
  percent_get(id, box, t);
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
    percent_cancel(a->commands[1], a->box, a->general_info);
  else if (strcmp(a->commands[0], "pause") == 0)
    percent_pause(a->commands[1], a->box, a->general_info);
  else if (strcmp(a->commands[0], "resume") == 0)
    percent_resume(a->commands[1], a->box, a->general_info);
  return (NULL);
}

void		*unload(void *args)
{
  INFO("Percent Plugin Unloaded");
  return (NULL);
}
