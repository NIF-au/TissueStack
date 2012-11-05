#include "core.h"

void		task_finished(char *task_id, t_tissue_stack *t)
{
  int		fi;
  int		i = 1;
  char		buff[18];
  int		j = 0;
  char		*path_task_id = NULL;
  int		fd;

  if (t && t->tasks)
    {
      if ((fd = open(t->tasks->path, O_RDWR)) > 0)
	{
	  if ((fi = open(t->tasks->path_tmp, (O_CREAT | O_RDWR | O_TRUNC), 0666)) > 0)
	    {
	      while (i > 0)
		{
		  memset(buff, 0, 18);
		  i = read(fd, buff, 17);
		  buff[i] = '\0';
		  FATAL("%i /\\/\\/\\/\\/\\/\\ %s", i, buff);
		  if (j > 0)
		    write(fi, buff, i);
		  j++;
		}
	      close(fd);
	      rename(t->tasks->path_tmp, t->tasks->path);
	      t->tasks->is_running = FALSE;
	      asprintf(&path_task_id, "%s/%s", t->percent->path, task_id);
	      if (path_task_id)
		free(path_task_id);
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
      FATAL("####### %i", i);
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
      t->plug_actions(t, dest, NULL);
      i = 0;
      while (i < 7)
	{
	  free(result[i]);
	  i++;
	}
      free(result);
    }
}

void		task_lunch(t_tissue_stack *t)
{
  char		buff[4096];
  int		len = 0;
  int		fd = 0;

  FATAL("%i", t->tasks->is_running);
  pthread_mutex_lock(&t->tasks->mutex);
  if (t && t->tasks && t->tasks->is_running == FALSE)
    {
      FATAL("=====================> inside lunch <=================");
      if ((fd = open(t->tasks->path, O_RDWR)) > 0)
	{
	  lseek(fd, SEEK_SET, 0);
	  len = read(fd, buff, 16);
	  if (len > 0)
	    {
	      buff[16] = '\0';
	      task_exec(buff, t);
	    }
	  close(fd);
	}
    }
  pthread_mutex_unlock(&t->tasks->mutex);
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
