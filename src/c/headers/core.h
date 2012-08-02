#ifndef __TISSUE_STACK_CORE__
#define __TISSUE_STACK_CORE__

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <minc2.h>

#include "thread_pool.h"

typedef struct		s_args_plug	t_args_plug;
typedef struct		s_plugin	t_plugin;
typedef struct		s_function	t_function;
typedef struct		s_tissue_stack	t_tissue_stack;
typedef struct		s_vol		t_vol;
typedef struct		s_char_prompt	t_char_prompt;
typedef struct		s_hist_prompt	t_hist_prompt;
typedef struct		s_error		t_error;

struct			s_error
{
  int			signal;
  t_plugin		*plugin;
  struct tm		*time;
  int			id;
  t_error		*next;
};

struct			s_plugin
{
  int			error;
  unsigned int	       	busy;
  pthread_t		thread_id;
  char			**start_command;
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
  void			*box;
  char			*name;
  char			*path;
  t_plugin		*this;
  t_tissue_stack	*general_info;
  void			(*destroy)(t_args_plug *a);
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
  pthread_cond_t	main_cond;
  pthread_mutex_t	main_mutex;
  pthread_t		main_id;
  t_error		*first_error;
  t_function		*functions;
  t_vol			*volume_first;
  t_plugin		*first;
  t_thread_pool		*tp;
  t_char_prompt		*prompt_first;
  t_hist_prompt		*hist_first;
  t_vol			*(*get_volume)(char *path, t_tissue_stack *general);
  t_vol			*(*check_volume)(char *path, t_tissue_stack *general);
  void			(*plug_actions)(t_tissue_stack *general, char *commands, void *box);
  void			(*clean_quit)(t_tissue_stack *t);
};

struct			s_vol
{
  mihandle_t		minc_volume;
  int			dim_nb;
  midimhandle_t		*dimensions;
  unsigned int		*size;
  double		*starts;
  double		*steps;
  char			*path;
  unsigned int		slices_max;
  char			**dim_name;
  int			raw_data;
  int			*dim_offset;
  int			*slice_size;
  int			raw_fd;
  t_vol			*next;
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

int             word_count(char *buff);
int             letter_count(char *buff, int position);
char            *str_n_cpy(char *str, int position, int len);
char            **str_to_wordtab(char *buff);
char            **copy_args(int start, char **commands);
void            change_tty_attr();
t_char_prompt   *get_current_position_prompt(t_tissue_stack *general);
void            aff_prompt(t_tissue_stack *general);
int             get_position_nb(t_tissue_stack *general);
void            delete_node(t_tissue_stack *general);
void            prompt_backspace(t_tissue_stack *general);
void            clear_prompt_list(t_tissue_stack *general);
int             prompt_len(t_tissue_stack *general);
void            add_history(t_tissue_stack *general);
void            prompt_enter(t_tissue_stack *general);
void            prompt_left(t_tissue_stack *general);
void            prompt_right(t_tissue_stack *general);
void            get_from_history(t_tissue_stack *general, int key);
void            prompt_up(t_tissue_stack *general);
void            prompt_down(t_tissue_stack *general);
int             action_keys(char *buff, t_tissue_stack *general);
void            stringify_char(char buff, t_tissue_stack *general);
void            prompt_start(t_tissue_stack *general);
t_args_plug     *create_plug_args(char **commands, t_tissue_stack *general, void *box);
t_args_plug	*create_copy_args(t_args_plug *args);
void            prompt_exec(char **commands, t_tissue_stack *general, void *box);

void            free_all_history(t_tissue_stack *t);
void            free_all_prompt(t_tissue_stack *t);

void		destroy_plug_args(t_args_plug *a);
void		destroy_command_args(char ** args);

/*		plugin.c		*/

void		list_plugins(t_tissue_stack *t, char *command);
void            plug_actions_from_external_plugin(t_tissue_stack *general, char *commands, void *box);
t_plugin        *get_plugin_by_name(char *name, t_plugin *first);
void            *plugin_load(void *a);
void            *plugin_start(void *a);
void            *plugin_unload(void *a);
void		*plugin_try_start(void *a);
void            free_all_plugins(t_tissue_stack *t);

/*		volume.c		*/

int		init_volume(t_vol *volume, char *path);
void		*file_actions(void *args);
void		list_volumes(t_tissue_stack *t, char *options);
void		add_volume(char *path, t_tissue_stack *t);
t_vol		*get_volume(char *path, t_tissue_stack *t);
t_vol		*check_volume(char *path, t_tissue_stack *t);
void		remove_volume(char *path, t_tissue_stack *t);
void		free_volume(t_vol *v);
void		free_all_volumes(t_tissue_stack *t);

/*		core.c			*/

unsigned int    get_slices_max(t_vol *volume);
void            init_func_ptr(t_tissue_stack *t);
void            init_prog(t_tissue_stack *t);

/*		error.c			*/

void		add_error(t_tissue_stack  *general, int signal, t_plugin *plug);
void		remove_error_by_id(t_tissue_stack *general, int id);
int		get_errors_nb_by_plugin(t_tissue_stack *general, t_plugin * plug);
int		*get_error_by_plugin(t_tissue_stack *general, t_plugin *plug);
void		clean_error_list(t_tissue_stack *general, int min);


#define X 0
#define Y 1
#define Z 2

#define ERROR_MAX 5
#define CLEANING_ERROR_TIME 30

#endif /* __TISSUE_STACK_CORE__ */
