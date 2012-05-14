#ifndef	__THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	struct		s_queue
{
  void			*(*function)(void *);
  void			*argument;
  struct s_queue	*next;
}			t_queue;

typedef struct		s_thread_pool
{
  pthread_t		*threads;
  pthread_cond_t	condvar;
  pthread_mutex_t	lock;
  pthread_cond_t	condvar_main;
  pthread_mutex_t	lock_main;
  t_queue		*first;
  t_queue		*last;
  int			nb_workers;
  int			tasks_to_do;
}			t_thread_pool;

void            thread_pool_free(t_thread_pool *p);
void            thread_pool_destroy(t_thread_pool *p);
void            thread_pool_init(t_thread_pool *p, unsigned int nb_threads);
void            thread_pool_add_task(void *(*function)(void *), void *args, t_thread_pool *p);
void            *worker_start(void *pool);


#endif	/* __THREAD_POOL_H__ */
