#include "thread_pool.h"

void		*worker_start(void *pool)
{
  t_queue	*task;
  t_thread_pool	*p;

  p = (t_thread_pool*)pool;
  while (p->loop)
    {
      task = NULL;
      // lock mutex to avoid concurrent access
      pthread_mutex_lock(&p->lock);
      if (p->tasks_to_do == 0)
	pthread_cond_wait(&(p->condvar), &(p->lock));
      // get the task and delete the task from the queue
      if (p->first != NULL)
	{
	  task = p->first;
	  if (p->first && p->first->next == NULL)
	    p->last = NULL;
	  p->first = p->first->next;
	  p->tasks_to_do--;
	  if (p->tasks_to_do == 0)
	    {
	      p->last = NULL;
	      p->first = NULL;
	    }
	}
      // unlock the mutex locked before
      pthread_mutex_unlock(&(p->lock));
      if (task != NULL)
	{
	  task->function(task->argument);
	  free(task);
	}
    }
  return (NULL);
}

void		thread_pool_add_task(void *(*function)(void *), void *args, t_thread_pool *p)
{
  t_queue	*tmp;

  pthread_mutex_lock(&(p->lock));
  if (p->last != NULL)
    {
      tmp = p->last;
      tmp->next = malloc(sizeof(*tmp->next));
      tmp = tmp->next;
      tmp->function = function;
      tmp->argument = args;
      p->last = tmp;
    }
  else
    {
      p->first = malloc(sizeof(*p->first));
      p->first->function = function;
      p->first->argument = args;
      p->last = p->first;
    }
  p->tasks_to_do++;
  if(pthread_cond_signal(&(p->condvar)) != 0)
    {
      write(2, "Error on condvar ThreadPool\n", strlen("Error on condvar ThreadPool\n"));
      exit(-1);
    }
  pthread_mutex_unlock(&(p->lock));
}

void		thread_pool_init(t_thread_pool *p, unsigned int nb_threads)
{
  int		i;

  i = 0;
  p->first = NULL;
  p->last = NULL;
  p->loop = 1;
  p->tasks_to_do = 0;
  p->nb_workers = nb_threads;
  p->add = thread_pool_add_task;
  // malloc the number of threads which we want in the threadpool
  p->threads = malloc(nb_threads * sizeof(*p->threads));
  // init mutex and cond var
  if ((pthread_mutex_init(&(p->lock), NULL) != 0) ||
      (pthread_cond_init(&(p->condvar), NULL) != 0) ||
      p->first != NULL ||
      p->threads == NULL)
    {
      write(2, "Error on init ThreadPool\n", strlen("Error on init ThreadPool\n"));
      exit(-1);
    }
  // init and luch threads
  while (i < nb_threads)
    {
      if (pthread_create(&(p->threads[i]), NULL, worker_start, (void*)p) != 0)
	{
	  write(2, "Error on init Thread workers\n", strlen("Error on init Thread workers\n"));
	  exit(-1);
	}
      i++;
    }
}

void		thread_pool_destroy(t_thread_pool *p)
{
  int		i;

  i = 0;
  // wake up all threads
  if (pthread_cond_broadcast(&(p->condvar)) != 0)
    {
      write(2, "Error pthread_cond_broadcast\n", strlen("Error pthread_cond_broadcast\n"));
      exit(-1);
    }
  // destroy threads
  usleep(1000);
  while (i < p->nb_workers)
    {
      if (pthread_detach(p->threads[i]) != 0)
	{
	  write(2, "Error pthread_join\n", strlen("Error pthread_join\n"));
	  exit(-1);
	}
      i++;
    }
  usleep(1000);
  // destroy condvar and mutex
  pthread_mutex_destroy(&(p->lock));
  pthread_cond_destroy(&(p->condvar));
  // free the rest
  //free(p->threads);
  thread_pool_free(p);
  free(p);
}

void		thread_pool_free(t_thread_pool *p)
{
  t_queue	*tmp;

  tmp = p->first;
  // free queue
  while (p->first != NULL)
    {
      tmp = p->first;
      p->first = p->first->next;
      free(tmp);
    }
  // free threads
  free(p->threads);
}
