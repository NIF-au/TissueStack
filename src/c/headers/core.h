#ifndef __TISSUE_STACK_CORE__
#define __TISSUE_STACK_CORE__

#define _GNU_SOURCE

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/prctl.h>
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

#include <sys/prctl.h>


#include "utils.h"
#include "thread_pool.h"
#include "tile_requests.h"
#include "memory_mapping.h"

#include "gtk/gtk.h"

typedef struct		s_args_plug	t_args_plug;
typedef struct		s_plugin	t_plugin;
typedef struct		s_function	t_function;
typedef struct		s_tissue_stack	t_tissue_stack;
typedef struct		s_vol		t_vol;
typedef struct		s_char_prompt	t_char_prompt;
typedef struct		s_hist_prompt	t_hist_prompt;
typedef struct		s_error		t_error;
typedef struct		s_nc_action	t_nc_action;
typedef struct		s_nc_func	t_nc_func;
typedef struct		s_log		t_log;
typedef struct		s_log_plug_fd	t_log_plug_fd;
typedef struct		s_log_level_fd	t_log_level_fd;
typedef struct		s_log_info_list	t_log_info_list;
typedef struct		s_log_plugin	t_log_plugin;


typedef	struct		s_percent_elem	t_percent_elem;
typedef	struct		s_time_elem	t_time_elem;
typedef	struct		s_time_tps	t_time_tps;
typedef	struct		s_prcnt_t	t_prcnt_t;
typedef struct		s_func_prcnt_t	t_func_prcnt_t;

struct			s_func_prcnt_t
{
  char			*name;
  void			(*func)(char **commands, void *box, t_tissue_stack *t);
};

struct			s_prcnt_t
{
  t_percent_elem	*first_percent;
  t_time_elem		*first_time;
  t_func_prcnt_t	*percent_func;
  t_func_prcnt_t	*time_func;
  int			files_writing;
  char			*path;
};

struct			s_percent_elem
{
  char			*id;
  t_time_tps		*time;
  int			total_blocks;
  int			blocks_done;
  float			percent;
  char			*filename;
  t_percent_elem	*next;
};

struct			s_time_tps
{
  time_t		start_time;
  time_t		end_time;
};

struct			s_time_elem
{
  char			*id;
  t_time_tps		*time;
  t_time_elem		*next;
};

/////////////////////////////////////////////////////////////////

struct			s_log_plugin
{
  t_tissue_stack	*tss;
  int			id;
};

struct			s_log_level_fd
{
  int			level;
  int			fd;
  t_log_level_fd	*next;
};

struct			s_log_plug_fd
{
  t_plugin		*plugin;
  int			fd;
  t_log_plug_fd		*next;
};

struct			s_log
{
  char			*path;
  int			debug;
  int			verbose;
  int			write_on_files;
  int			write_on_plug_files;
  int			write_on_level_files;
  int			general_fd;
  int			state;
  t_log_plug_fd		*first_plug_fd;
  t_log_level_fd	*first_level_fd;
};

struct			s_nc_func
{
  void			(*action)(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t);
  t_nc_func		*next;
};

struct			s_nc_action
{
  char			*name;
  t_nc_func		*first_func;
  t_nc_action		*next;
};


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
  int			id;
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
  t_log			*log;
  t_char_prompt		*prompt_first;
  t_hist_prompt		*hist_first;
  t_tile_requests 	*tile_requests;
  t_memory_mapping 	*memory_mappings;
  t_nc_action		*first_notification;
  t_prcnt_t		*percent;
  void			(*percent_get)(char **buff, char *id, t_tissue_stack *t);
  void			(*percent_add)(int blocks, char *id, t_tissue_stack *t);
  void			(*percent_init)(int total_blocks, char **id, char *filename, t_tissue_stack *t);
  t_vol			*(*get_volume)(char *path, t_tissue_stack *general);
  t_vol			*(*check_volume)(char *path, t_tissue_stack *general);
  void			(*plug_actions)(t_tissue_stack *general, char *commands, void *box);
  void			(*clean_quit)(t_tissue_stack *t);
  void			(*create_notification)(char *name,
						  void (*action)(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t), t_tissue_stack *t);
  int			(*raise)(int id, char *name, char *command, void *data, t_tissue_stack *t);
  int			(*subscribe)(char *name, void (*action)(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t), t_tissue_stack *t);
};

struct			s_vol
{
  mihandle_t		minc_volume;		// pointer on a MINC structure containig the MINC volume information.
  int			dim_nb;			// number of dimensions presents into the file
  midimhandle_t		*dimensions;		// array[dimension] internal MINC pointer of the dimension
  unsigned int		*size;			// array[dimension] containig the size of each dimension (in slices)
  double		*starts;		// array[dimension] containig the starts of each dimension
  double		*steps;			// array[dimension] containig the steps between each voxel
  char			*path;			// file path
  unsigned int		slices_max;		// total number of slices presents inside of the volume
  char			**dim_name;		// array[dimension] containing the name of each dimension
  char			*dim_name_char;		// array[dimension] containing the firts character of the dimension name
  int			raw_data;		// flag in order to know if is a MINC or a RAW
  unsigned long long int	*dim_offset;	// array[dimension] containing the offset of the beginning of each dimension
  int			*slice_size;		// array[dimension] containing the size of a slice in each dimension
  int			raw_fd;			// File descriptor associate to a raw file
  double		min;                    // min value of the volume
  double		max;                    // max value of the volume
  unsigned char		color_range_min;	// min value of the volume color range
  unsigned char		color_range_max;	// max value of the volume color range
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
t_plugin	*get_plugin_by_id(int id, t_tissue_stack *t);
t_plugin	*plugindup(t_plugin *p);
void            *plugin_load(void *a);
void            *plugin_start(void *a);
void            *plugin_unload(void *a);
void		*plugin_try_start(void *a);
void            free_all_plugins(t_tissue_stack *t);
void 		destroy_t_plugin(t_plugin * this, t_tissue_stack * general);
void		plugin_load_from_string(char *str, t_tissue_stack *t);
void		plugin_start_from_string(char *str, t_tissue_stack *t);

/*		volume.c		*/

int		init_volume(t_memory_mapping * memory_mappings, t_vol *volume, char *path);
void		*file_actions(void *args);
void		list_volumes(t_tissue_stack *t, char *options);
void		add_volume(char *path, t_tissue_stack *t);
t_vol		*get_volume(char *path, t_tissue_stack *t);
t_vol		*check_volume(char *path, t_tissue_stack *t);
t_vol  *load_volume(t_args_plug * a, char * path);
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

/*		percent_and_time		*/

int		is_num(char *str);
void		percent_time_write(char *str, char **commands, void *box);
void		percent_init_direct(int total_blocks, char **id, char *filename, t_tissue_stack *t);
t_percent_elem	*get_percent_elem_by_id(char *id, t_prcnt_t *p);
void		percent_add_direct(int blocks, char *id, t_tissue_stack *t);
void		percent_get_direct(char **buff, char *id, t_tissue_stack *t);
void		percent_destroy(char **commands, void *box, t_tissue_stack *t);
void		init_percent_time(t_tissue_stack *t, char *path);

/*		notification_center.c		*/

void		nc_create_notification(char *name,
				       void (*action)(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t),
				       t_tissue_stack *t);
t_nc_action	*nc_get_action_by_name(char *name, t_tissue_stack *t);
int		nc_raise(int id, char *name, char *command, void *data, t_tissue_stack *t);
int		nc_subscribe(char *name, void (*action)(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t),
			     t_tissue_stack *t);
void		nc_list(t_tissue_stack *t);

/*		log_center.c			*/

char		*concat_path(char *root, char *path, char *extension);
void            lc_add_log_to_info_list(char *name, t_plugin *plugin, char *command, t_tissue_stack *t);
void            lc_write_on_plug_fd(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t);
void            lc_write_on_level_fd(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t);
void            lc_write_on_general_fd(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t);
void            lc_debug(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t);
void            lc_info(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t);
void            lc_warning(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t);
void            lc_error(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t);
void            lc_fatal(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t);

#define X 0
#define Y 1
#define Z 2

#define ON 1
#define OFF 0

t_log_plugin		log_plugin;

#define LOG_INIT(a) {				\
    log_plugin.tss = a->general_info;		\
    log_plugin.id = a->this->id;		\
  }

#define LOG_PROCESS(level_name, message, args...) {			\
    char		*tmp, *tmp2;					\
  									\
    asprintf(&tmp, message, ## args);					\
    if (log_plugin.tss->log->debug == ON) {				\
      asprintf(&tmp2, "%s | %s | %d", tmp, __FILE__, __LINE__);		\
      free(tmp);							\
      log_plugin.tss->raise(log_plugin.id, level_name, (char*)tmp2, NULL, log_plugin.tss); \
    }									\
    else								\
      log_plugin.tss->raise(log_plugin.id, level_name, (char*)tmp, NULL, log_plugin.tss); \
  }

#define DEBUG(message, args...) {					\
    LOG_PROCESS("log_debug", message, ## args);				\
  }

#define INFO(message, args...) {					\
    LOG_PROCESS("log_info", message, ## args);				\
  }

#define WARNING(message, args...) {					\
    LOG_PROCESS("log_warning", message, ## args);				\
  }

#define ERROR(message, args...) {					\
    LOG_PROCESS("log_error", message, ## args);				\
  }

#define FATAL(message, args...) {					\
    LOG_PROCESS("log_fatal", message, ## args);				\
  }


#define LOG(level, message, args...) {					\
    if (level == 0)							\
      DEBUG(message, ## args);						\
    else if (level == 1)						\
      INFO(message, ## args);						\
    else if (level == 2)						\
      WARNING(message, ## args);						\
    else if (level == 3)						\
      ERROR(message, ## args);						\
    else if (level == 4)						\
      FATAL(message, ## args);						\
  }

#define ERROR_MAX 5
#define CLEANING_ERROR_TIME 30

#endif /* __TISSUE_STACK_CORE__ */
