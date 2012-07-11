/*
** core.c for hello in /home/oliver/workspace/TissueStack/src/c
**
** Made by Oliver Nicolini
** E-Mail   o.nicolini@uq.edu.au
**
** Started on  Mon May 21 13:05:15 2012 Oliver Nicolini
** Last update Tue Jul 10 12:19:31 2012 Oliver Nicolini
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
    printf("Signal : %i\n", sig);

    switch (sig) {
		case SIGHUP:
		case SIGQUIT:
		case SIGTERM:
		case SIGINT:
			t_global->clean_quit(t_global);
			break;
	}

  /*
  t_plugin		*tmp;
  pthread_t		id;
  int			errors;
  char			command[200];
  char			*name;
  char			*start_command;
  char			*path;

  id = pthread_self();
  tmp = t_global->first;
  while (tmp != NULL)
    {
      if (tmp->thread_id == id)
	{
	  add_error(t_global, sig, tmp);
	  fprintf(stderr, "The thread hosting the plugin: %s - received the signal %i\n", tmp->name, sig);
	  break;
	}
      tmp = tmp->next;
    }
  errors = get_errors_nb_by_plugin(t_global, tmp);
  if (errors >= ERROR_MAX)
    {
      fprintf(stderr, "The plugin %s locate at %s haz crashed several times. This plugin is now disabled\n", tmp->name, tmp->path);
      sprintf(command, "unload %s", tmp->name);
      t_global->plug_actions(t_global, command, NULL);
    }
  else
    {
      path = strdup(tmp->path);
      name = strdup(tmp->name);
      start_command = from_array_to_string(tmp->start_command);
      sprintf(command, "unload %s", tmp->name);
      t_global->plug_actions(t_global, command, NULL);
      usleep(1000);
      memset(command, 0, 200);
      sprintf(command, "load %s %s", name, path);
      t_global->plug_actions(t_global, command, NULL);
      usleep(1000);
      memset(command, 0, 200);
      sprintf(command, "start %s %s ", name, start_command);
      t_global->plug_actions(t_global, command, NULL);
    }
    clean_error_list(t_global, CLEANING_ERROR_TIME);*/
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
  t->plug_actions = plug_actions_from_external_plugin;
  t->get_volume = get_volume;
  t->check_volume = check_volume;
  t->clean_quit = clean_quit;
  t->first = NULL;
  pthread_cond_init(&t->main_cond, NULL);
  pthread_mutex_init(&t->main_mutex, NULL);
  init_func_ptr(t);
}

void		free_core_struct(t_tissue_stack *t)
{
  printf("\nFreeing\n");
  free_all_volumes(t);
  free_all_plugins(t);
  free_all_history(t);
  free_all_prompt(t);
  free(t->functions);
  free(t);
}

int		main(int argc, char **argv)
{
  int			result;
  t_tissue_stack	*t;
  char			serv_command[20];

  // initialisation of some variable
  t = malloc(sizeof(*t));
  init_prog(t);
  // intitialisation the volume
  if (argc > 2)
    {
      if (argv[2] != NULL && strcmp(argv[2], "--prompt") != 0)
	{
	  t->volume_first = malloc(sizeof(*t->volume_first));
	  if ((result = init_volume(t->volume_first, argv[2])) != 0)
	    return (result);
	}
      else if (argv[3] != NULL && strcmp(argv[3], "--prompt") != 0)
	{
	  t->volume_first = malloc(sizeof(*t->volume_first));
	  if ((result = init_volume(t->volume_first, argv[3])) != 0)
	    return (result);
	}
    }
  else
    t->volume_first = NULL;

  // lunch thread_pool
  t->tp = malloc(sizeof(*t->tp));
  thread_pool_init(t->tp, 10);
  (t->plug_actions)(t, "load png /usr/local/plugins/TissueStackPNGExtract.so", NULL);
  sleep(1);
  (t->plug_actions)(t, "load serv /usr/local/plugins/TissueStackCommunicator.so", NULL);
  sleep(2);
  sprintf(serv_command, "start serv %s", argv[1]);
  (t->plug_actions)(t, serv_command, NULL);
  (t->plug_actions)(t, "load comm /usr/local/plugins/TissueStackProcessCommunicator.so", NULL);
  sleep(2);
  (t->plug_actions)(t, "start comm", NULL);
  signal_manager(t);
  if ((argv[2] != NULL && strcmp(argv[2], "--prompt") == 0) ||
      (argv[3] != NULL && strcmp(argv[3], "--prompt") == 0))
    prompt_start(t);
  else
    {
      printf("TissueStackImageServer Running\n");
      pthread_mutex_lock(&t->main_mutex);
      pthread_cond_wait(&t->main_cond, &t->main_mutex);
      pthread_mutex_unlock(&t->main_mutex);
    }

  // free all the stuff mallocked
  printf("Shutting down ...\n");

  t->tp->loop = 0;
  thread_pool_destroy(t->tp);
  free_core_struct(t);

  printf("Good Bye\n");

  return (0);
}
