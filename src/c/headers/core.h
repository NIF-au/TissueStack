#ifndef __MINC_TOOL_CORE__
#define __MINC_TOOL_CORE__

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <minc2.h>

#include <termios.h>
#include <sys/ioctl.h>

#include "thread_pool.h"

typedef struct		s_args_plug	t_args_plug;
typedef struct		s_plugin	t_plugin;
typedef struct		s_function	t_function;
typedef struct		s_tissue_stack	t_tissue_stack;
typedef struct		s_vol		t_vol;
typedef struct		s_char_prompt	t_char_prompt;
typedef struct		s_hist_prompt	t_hist_prompt;

struct			s_plugin
{
  int			error;
  unsigned int	       	busy;
  char			*name;
  char			*path;
  void			*handle;
  void			*stock;
  t_plugin		*next;
  t_plugin		*prev;
};

struct			s_args_plug
{
  char			**commands;
  char			*name;
  char			*path;
  t_plugin		*this;
  t_tissue_stack	*general_info;
};

struct			s_function
{
  char			*name;
  void			*(*ptr)(void *a);
};

struct			s_tissue_stack
{
  int			nb_func;
  int			quit;
  t_function		*functions;
  t_vol			*volume;
  t_plugin		*first;
  t_thread_pool		*tp;
  t_char_prompt		*prompt_first;
  t_hist_prompt		*hist_first;
};

struct			s_vol
{
  mihandle_t		minc_volume;
  int			dim_nb;
  midimhandle_t		*dimensions;
  unsigned int		*size;
  char			*path;
  unsigned int		slices_max;
};

struct			s_char_prompt
{
  char			c;
  unsigned int		position;
  t_char_prompt		*next;
  t_char_prompt		*prev;
};

struct			s_hist_prompt
{
  char			*commands;
  unsigned int		position;
  t_hist_prompt		*next;
  t_hist_prompt		*prev;
};

/*		prompt.c		*/

void            prompt_start(t_tissue_stack *general);
char            **prompt_str_to_wordtab(char *buff);
t_args_plug     *create_plug_args(char **commands, t_tissue_stack *general);
void            prompt_exec(char **commands, t_tissue_stack *general);

/*		plugin.c		*/

t_plugin        *get_plugin_by_name(char *name, t_plugin *first);
void            *plugin_load(void *a);
void            *plugin_start(void *a);
void            *plugin_unload(void *a);

/*		core.c			*/

void            init_func_ptr(t_tissue_stack *t);
void            init_prog(t_tissue_stack *t, char *path_arg, int nb_args);
int             init_volume(t_vol *volume);

#define print(mutex, format, ...) {					\
  pthread_mutex_lock(&mutex);						\
  printf(format, __VA_ARGS__);						\
  pthread_mutex_unlock(&mutex);						\
}

#define X 0
#define Y 1
#define Z 2

#endif /* __MINC_TOOL_CORE__ */
