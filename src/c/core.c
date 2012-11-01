/*
** core.c for hello in /home/oliver/workspace/TissueStack/src/c
**
** Made by Oliver Nicolini
** E-Mail   o.nicolini@uq.edu.au
**
** Started on  Mon May 21 13:05:15 2012 Oliver Nicolini
** Last update Thu Nov  1 15:19:23 2012 Oliver Nicolini
*/

#include "core.h"

static t_tissue_stack	*t_global;

char			*from_array_to_string(char **array)
{
  int			i;
  int			j;
  int			k;
  char			*dest;

  i = 0;
  k = 0;
  while (array[i] != NULL)
    {
      j = 0;
      while (array[i][j] != '\0')
	{
	  k++;
	  j++;
	}
      k++;
      i++;
    }
  dest = malloc((k + 1) * sizeof(*dest));
  i = 0;
  while (array[i] != NULL)
    {
      if (i != 0)
	strcat(dest, " ");
      strcat(dest, array[i]);
      i++;
    }
  return (dest);
}

void			signal_handler(int sig)
{
    WARNING("Received Signal : %i", sig);

    switch (sig) {
		case SIGHUP:
		case SIGQUIT:
		case SIGTERM:
		case SIGINT:
			t_global->clean_quit(t_global);
			break;
	}
}

void			signal_manager(t_tissue_stack *t)
{
  struct sigaction	act;
  int			i;


  t_global = t;
  act.sa_handler = signal_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  i = 1;
  while (i < 32)
    {
      if (i != 11)
	sigaction(i, &act, NULL);
      i++;
    }
}

unsigned int            get_slices_max(t_vol *volume)
{
  // get the larger number of slices possible
  if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[X]))
    {
      if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[Y]))
        return (volume->size[X] * volume->size[Y]);
    }
  else if ((volume->size[Z] * volume->size[X]) > (volume->size[Z] * volume->size[Y]))
    return (volume->size[Z] * volume->size[X]);
  return (volume->size[Z] * volume->size[Y]);
}

void		*list_actions(void *args)
{
  t_args_plug   *a;

  a = (t_args_plug*)args;
  if (strcmp(a->name, "volumes") == 0)
    list_volumes(a->general_info, a->commands[0]);
  else if (strcmp(a->name, "plugins") == 0)
    list_plugins(a->general_info, a->commands[0]);
  else if (strcmp(a->name, "help") == 0)
    printf("Usage; list [plugins | volumes] --verbose\n");
  return (NULL);
}

void		init_func_ptr(t_tissue_stack *t)
{
  t_function	*functions;

  functions = malloc(6 * sizeof(*functions));
  functions[0].name = "load";
  functions[1].name = "start";
  functions[2].name = "unload";
  functions[3].name = "file";
  functions[4].name = "list";
  functions[5].name = "try_start";
  functions[0].ptr = plugin_load;
  functions[1].ptr = plugin_start;
  functions[2].ptr = plugin_unload;
  functions[3].ptr = file_actions;
  functions[4].ptr = list_actions;
  functions[5].ptr = plugin_try_start;
  t->functions = functions;
  t->nb_func = 6;
}

void		clean_quit(t_tissue_stack *t)
{
  pthread_cond_signal(&t->main_cond);
}

void            init_prog(t_tissue_stack *t)
{
  //char		*path = NULL;

  t->plug_actions = plug_actions_from_external_plugin;

  t->percent_init = percent_init_direct;
  t->percent_add = percent_add_direct;
  t->percent_get = percent_get_direct;
  t->percent_cancel = percent_cancel_direct;
  t->is_percent_paused_cancel = is_percent_paused_cancel;
  t->clean_pause_queue = clean_pause_queue;

  t->percent_pause = percent_pause_direct;
  t->percent_resume = percent_resume_direct;

  t->tasks = malloc(sizeof(*t->tasks));

  t->tasks->f = NULL;
  t->tasks->add_to_queue = task_add_queue;
  t->tasks->path = strdup(CONCAT_APP_PATH("tasks/general"));
  t->tasks->path_tmp = strdup(CONCAT_APP_PATH("tasks/general.tmp"));
  t->tasks->is_running = FALSE;
  pthread_mutex_init(&t->tasks->mutex, NULL);
  pthread_mutex_init(&t->tasks->queue_mutex, NULL);


  t->tile_requests = malloc(sizeof(*t->tile_requests));
  init_tile_requests(t->tile_requests);
  t->memory_mappings = malloc(sizeof(*t->memory_mappings));
  init_memory_mapping(t->memory_mappings);
  t->get_volume = get_volume;
  t->check_volume = check_volume;
  t->clean_quit = clean_quit;
  t->first = NULL;
  t->first_notification = NULL;
  pthread_cond_init(&t->main_cond, NULL);
  pthread_mutex_init(&t->main_mutex, NULL);
  nc_create_notification("log_debug",	lc_debug,   t);
  nc_create_notification("log_info",	lc_info,    t);
  nc_create_notification("log_warning", lc_warning, t);
  nc_create_notification("log_error",	lc_error,   t);
  nc_create_notification("log_fatal",	lc_fatal,   t);
  t->create_notification = nc_create_notification;
  t->subscribe = nc_subscribe;
  t->raise = nc_raise;
  t->log = malloc(sizeof(*t->log));
  t->log->state = ON;
  t->log->path = strdup(CONCAT_APP_PATH("logs/"));
  t->log->debug = OFF;
  t->log->verbose = OFF;
  t->log->write_on_files = ON;
  t->log->write_on_plug_files = OFF;
  t->log->write_on_level_files = ON;
  log_plugin.id = pthread_self();
  log_plugin.tss = t;
  if (t->log->state)
    {
      // make sure directory exists !
      t_string_buffer * actualPath = createDirectory(t->log->path, 0766);
      // couldn't create directory
      if (actualPath == NULL)
	{
	  ERROR("Couldn't create %s", t->log->path);
	  t->log->state = OFF; // turn logging off
	} else
	{
	  //path = concat_path(actualPath->buffer, "tss-general", ".log");
	  free_t_string_buffer(actualPath);
	}

      /*
	if ((t->log->general_fd = open(path, O_CREAT | O_RDWR | O_TRUNC)) == -1)
	{
	  ERROR("Open %s failed", path);
	  t->log->state = OFF; // turn logging off
	}
	stat(path, &results);
	if (results.st_mode != 0666)
	{

	  if (chmod(path, 0666) == -1)
	    {
	      ERROR("Chmod 666  %s failed", path);
	      t->log->state = OFF; // turn logging off
	    }
	  stat(path, &results);
	  }*/
    }
  init_func_ptr(t);
  init_percent_time(t, CONCAT_APP_PATH("tasks/"));
}

void		free_core_struct(t_tissue_stack *t)
{
  if (t == NULL) return;

  INFO("Freeing Allocated Resources...");
  free_all_volumes(t);
  free_all_history(t);
  free_all_plugins(t);
  free_all_prompt(t);
  free_all_notifications(t);
  free_all_percent(t);
  free_all_tasks(t);
  free_all_log(t);
  if (t->tile_requests != NULL) t->tile_requests->destroy(t->tile_requests);
  if (t->memory_mappings != NULL) {
    destroy_memory_mapping(t->memory_mappings);
    free(t->memory_mappings);
  }
  free(t->functions);
  free(t);
}

int		main(int argc, char **argv)
{
  int			result;
  t_tissue_stack	*t;
  char			serv_command[20];

  prctl(PR_SET_NAME, "TS_CORE");

  // initialisation of some variable
  t = malloc(sizeof(*t));
  init_prog(t);
  srand((unsigned)time(NULL));
  // intitialisation the volume
  if (argc > 2)
    {
      if (argv[2] != NULL && strcmp(argv[2], "--prompt") != 0)
	{
	  t->volume_first = malloc(sizeof(*t->volume_first));
	  if ((result = init_volume(t->memory_mappings, t->volume_first, argv[2])) != 0)
	    return (result);
	}
      else if (argv[3] != NULL && strcmp(argv[3], "--prompt") != 0)
	{
	  t->volume_first = malloc(sizeof(*t->volume_first));
	  if ((result = init_volume(t->memory_mappings, t->volume_first, argv[3])) != 0)
	    return (result);
	}
    }
  else
    t->volume_first = NULL;


  // lunch thread_pool
  t->tp = malloc(sizeof(*t->tp));
  thread_pool_init(t->tp, 16);

  // These are the plugins that should be loaded by default.
  // Please no rash name changes since JNI asks for the predefined names!


  plugin_load_from_string("load image /usr/local/plugins/TissueStackImageExtract.so", t);
  plugin_load_from_string("load serv /usr/local/plugins/TissueStackCommunicator.so", t);
  plugin_load_from_string("load comm /usr/local/plugins/TissueStackProcessCommunicator.so", t);
  plugin_load_from_string("load minc_info /usr/local/plugins/TissueStackMincInfo.so", t);
  plugin_load_from_string("load minc_converter /usr/local/plugins/TissueStackMincConverter.so", t);
  plugin_load_from_string("load nifti_converter /usr/local/plugins/TissueStackNiftiConverter.so", t);
  plugin_load_from_string("load progress /usr/local/plugins/TissueStackPercent.so", t);


  sprintf(serv_command, "start serv %s", argv[1]);

  // start plugins
  (t->plug_actions)(t, serv_command, NULL);
  (t->plug_actions)(t, "start comm", NULL);

  signal_manager(t);

  if ((argv[2] != NULL && strcmp(argv[2], "--prompt") == 0) ||
      (argv[3] != NULL && strcmp(argv[3], "--prompt") == 0))
    prompt_start(t);
  else
    {
      INFO("TissueStackImageServer Running!");
      pthread_mutex_lock(&t->main_mutex);
      pthread_cond_wait(&t->main_cond, &t->main_mutex);
      pthread_mutex_unlock(&t->main_mutex);
    }

  // free all the stuff mallocked
  INFO("Shutting down TissueStackImageServer!");

  t->tp->loop = 0;
  thread_pool_destroy(t->tp);
  free_core_struct(t);

  return (0);
}
