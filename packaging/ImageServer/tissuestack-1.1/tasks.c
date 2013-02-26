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

void		task_finished(char *task_id, t_tissue_stack *t)
{
  int		fi;
  int		i = 1;
  int		count = 0;
  char		*buff;
  int		fd;

  if (t && t->tasks)
    {
      if ((fd = open(t->tasks->path, O_RDWR)) > 0)
	{
	  if ((fi = open(t->tasks->path_tmp, (O_CREAT | O_RDWR | O_TRUNC), 0666)) > 0)
	    {
	      buff = malloc(17 * sizeof(*buff));
	      while (i > 0)
		{
		  memset(buff, '\0', 17);
		  lseek(fd, count, SEEK_SET);
		  i = read(fd, buff, 16);
		  if (i > 0)
		    {
		      buff[i] = '\0';
		      if (strncmp(buff, task_id, 16) != 0)
			{
			  write(fi, buff, 16);
			  write(fi, "\n", 1);
			}
		    }
		  count += 17;
		}
	      free(buff);
	      close(fd);
	      rename(t->tasks->path_tmp, t->tasks->path);
	      if (t->tasks && t->tasks->task_id && strncmp(task_id, t->tasks->task_id, 16) == 0)
		{
		  t->tasks->is_running = FALSE;
		}
	      task_lunch(t);
	    }
	}
    }
}

void		task_add_queue(char *task_id, t_tissue_stack *t)
{
  static int	i = 0;
  int		fd = 0;

  pthread_mutex_lock(&t->tasks->queue_mutex);
  if (t != NULL && t->tasks != NULL && t->tasks->path != NULL)
    fd = open(t->tasks->path, (O_RDWR | O_CREAT | O_APPEND), 0666);
  if (t && t->tasks && fd > 0)
    {
      i++;
      lseek(fd, 0, SEEK_SET);
      write(fd, task_id, strlen(task_id));
      write(fd, "\n", 1);
      close(fd);
    }
  task_lunch(t);
  pthread_mutex_unlock(&t->tasks->queue_mutex);
}

void		task_exec(char *task_id, t_tissue_stack *t)
{
  FILE		*f;
  char		**result;
  int		i;
  char		*dest;

  if ((result = read_from_file_by_id(task_id, &f, t)) != NULL)
    {
      asprintf(&dest, "start %s %s %s", (result[4][0] == '0' ? "image" :
					 (result[4][0] == '1' ? "minc_converter" : "nifti_converter")),
	       result[6], task_id);
      t->tasks->is_running = TRUE;
      t->tasks->task_id = strdup(task_id);
      t->plug_actions(t, dest, NULL);
      i = 0;
      while (i < 7)
	{
	  free(result[i]);
	  i++;
	}
      free(result);
    }
  else
    {
      t->tasks->is_running = FALSE;
      task_finished(task_id, t);
    }
}

void		task_lunch(t_tissue_stack *t)
{
  char		*buff;
  int		len = 0;
  int		fd = 0;

  pthread_mutex_lock(&t->tasks->mutex);
  if (t && t->tasks && t->tasks->is_running == FALSE)
    {
      if ((fd = open(t->tasks->path, O_RDWR)) > 0)
	{
	  buff = malloc(17 * sizeof(*buff));
	  memset(buff, '\0', 17);
	  lseek(fd, SEEK_SET, 0);
	  if ((len = read(fd, buff, 16)) > 0)
	    {
	      buff[16] = '\0';
	      task_exec(buff, t);
	    }
	  close(fd);
	  free(buff);
	}
    }
  pthread_mutex_unlock(&t->tasks->mutex);
}

void		task_clean_up(t_tissue_stack *t)
{
  char		**result;
  FILE		*f;
  int		fd = 0;
  int		fi = 0;
  int		i = 1;
  char		*buff;
  int		count = 0;


  if (t && t->tasks && t->tasks->path)
    {
      if ((fd = open(t->tasks->path, O_RDWR)) > 0)
	{
	  if ((fi = open(t->tasks->path_tmp, (O_CREAT | O_RDWR | O_TRUNC), 0666)) > 0)
	    {

	      buff = malloc(17 * sizeof(*buff));
	      while (i > 0)
		{
		  memset(buff, '\0', 17);
		  lseek(fd, count, SEEK_SET);
		  i = read(fd, buff, 16);
		  if (i > 0)
		    {
		      buff[i] = '\0';
		      if ((result = read_from_file_by_id(buff, &f, t)) != NULL)
			{
			  if (strcmp(result[0], "100") != 0)
			    {
			      write(fi, buff, i);
			      write(fi, "\n", 1);
			    }
			  fclose(f);
			}
		    }
		  count += 17;
		}
	      free(buff);
	      close(fd);
	      rename(t->tasks->path_tmp, t->tasks->path);
	    }
	}
    }
}

void		free_all_tasks(t_tissue_stack *t)
{
  if (t->tasks != NULL)
    {
      if (t->tasks->path)
	free(t->tasks->path);
      if (t->tasks->path_tmp)
	free(t->tasks->path_tmp);
      if (t->tasks->task_id)
	free(t->tasks->task_id);
      if (t->tasks->f)
	fclose(t->tasks->f);
      free(t->tasks);
    }
}
