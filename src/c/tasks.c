#include "core.h"

void		task_finished(char *task_id, t_tissue_stack *t)
{
  FILE		*fi;
  int		i = 1;
  char		buff[18];
  int		j = 0;

  if (t && t->tasks && t->tasks->f != NULL)
    {
      FATAL("%x", (unsigned int)((size_t)((void*)t->tasks->f)));
      fclose(t->tasks->f);
      t->tasks->f = fopen(t->tasks->path, "r+");
      fi = fopen(t->tasks->path_tmp, "w+");
      while (i > 0)
	{
	  memset(buff, 0, 18);
	  i = fread(buff, 1, 17, t->tasks->f);
	  buff[i] = '\0';
	  FATAL("%i /\\/\\/\\/\\/\\/\\ %s", i, buff);
	  if (j > 0)
	    fwrite(buff, 1, i, fi);
	  j++;
	}
      fclose(t->tasks->f);
      t->tasks->f = fi;
      rename(t->tasks->path_tmp, t->tasks->path);
      t->tasks->is_running = FALSE;
      task_lunch(t);
    }
}

void		task_add_queue(char *task_id, t_tissue_stack *t)
{
  pthread_mutex_lock(&t->tasks->queue_mutex);
  if (t != NULL && t->tasks != NULL)
    t->tasks->f = fopen(t->tasks->path, "w+");
  if (t && t->tasks && t->tasks->f != NULL)
    {
      fseek(t->tasks->f, 0, SEEK_END);
      fwrite(task_id, 1, strlen(task_id), t->tasks->f);
      fwrite("\n", 1, 1, t->tasks->f);
      fflush(t->tasks->f);
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

  FATAL("YOP 2 ");
  result = read_from_file_by_id(task_id, &f, t);
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

void		task_lunch(t_tissue_stack *t)
{
  char		buff[4096];
  int		len = 0;

  FATAL("%i", t->tasks->is_running);
  pthread_mutex_lock(&t->tasks->mutex);
  if (t && t->tasks && t->tasks->is_running == FALSE)
    {
      t->tasks->f = fopen(t->tasks->path, "r+");
      fseek(t->tasks->f, SEEK_SET, 0);
      len = fread(buff, 1, 16, t->tasks->f);
      if (len > 0)
	{
	  buff[16] = '\0';
	  task_exec(buff, t);
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
