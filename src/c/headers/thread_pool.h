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
#ifndef	__THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>

typedef struct		s_thread_pool		t_thread_pool;

typedef	struct		s_queue
{
  void			*(*function)(void *);
  void			*argument;
  struct s_queue	*next;
}			t_queue;

struct			s_thread_pool
{
  pthread_t		*threads;
  pthread_cond_t	condvar;
  pthread_mutex_t	lock;
  t_queue		*first;
  t_queue		*last;
  int			nb_workers;
  int			tasks_to_do;
  int			loop;
  void			(*add)(void *(*function)(void *), void *args, t_thread_pool *p);
};

void            thread_pool_free(t_thread_pool *p);
void            thread_pool_destroy(t_thread_pool *p);
void            thread_pool_init(t_thread_pool *p, unsigned int nb_threads);
void            thread_pool_add_task(void *(*function)(void *), void *args, t_thread_pool *p);
void            *worker_start(void *pool);


#endif	/* __THREAD_POOL_H__ */
