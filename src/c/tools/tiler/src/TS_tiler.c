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

#include "core.h"
#include <getopt.h>

static t_tissue_stack	*t_global;

/* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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
      if (i != 11 && i != 2)
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
  t->memory_mappings = NULL;
  #ifdef USE_MM
  t->memory_mappings = malloc(sizeof(*t->memory_mappings));
  init_memory_mapping(t->memory_mappings);
  #endif
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

  #ifndef _TSS_DEBUG_
  t->log->debug = OFF;
  t->log->verbose = OFF;
  #else
  t->log->debug = ON;
  t->log->verbose = ON;
  #endif

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

    }
  init_func_ptr(t);
  init_percent_time(t, strdup(CONCAT_APP_PATH("tasks/")));
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
  }
  free(t->functions);
  free(t);
}

static char * PLUGINS[1][2] = {
		{"image", "TissueStackImageExtractPreTilingTool.so"},
};

//		{"progress", "TissueStackPercent.so"},

int		main(int argc, char **argv)
{
  int			nr_of_plugins, x;
  t_tissue_stack	*t;
  char			load_command[150];
  char			*tile_command;
  char			*preview_command;
  char			*file = NULL;
  char			*pathdir = NULL;
  int			level = 0;
  int			x_start = -1;
  int			x_end = -1;
  int			y_start = -1;
  int			y_end = -1;
  int			z_start = -1;
  int			z_end = -1;
  float			zoomlevel = 0; // 0 == 0.25, 1 == 0.5, 2 == 0.75, 3 == 1, 4 == 1.25, 5 == 1.50, 6 == 1.75 ....
  int			slice = 0;
  char			*dimension = NULL;
  int			c;
  // av[1] dimension
  // av[2] slice;
  // av[3] level;
  // av[4] pathdir
  // av[5] infile.raw

  while (1)
  {
  	static struct option long_options[] = {
               /* These options don't set a flag.
                  We distinguish them by their indices. */
  		{"dimension",	required_argument, 0, 'd'},
  		{"slice",		required_argument, 0, 's'},
  		{"level",  		required_argument, 0, 'l'},
  		{"path",  		required_argument, 0, 'p'},
  		{"file",    	required_argument, 0, 'f'},
  		{0, 0, 0, 0}
  	};
           /* getopt_long stores the option index here. */
  	int option_index = 0;
  	c = getopt_long (argc, argv, "d:s:l:p:f:", long_options, &option_index);
  	if (c == -1)
  		break;
  	switch (c)
  	{
  		case 'd':
  		dimension = strdup(optarg);
  		break;

  		case 's':
  		slice = atoi(optarg);
  		break;

  		case 'l':
  		level = atoi(optarg);
  		break;

  		case 'p':
  		pathdir = strdup(optarg);
  		break;

  		case 'f':
  		file = strdup(optarg);
  		break;

  		case '?':
            exit (0);   /* getopt_long already printed an error message. */
  		break;

  		default:
  		printf("Usage: %s -d DIMENSION_NAME -s SLICE_NB -l LEVEL -p PATH -f FILE\n", argv[0]);
  		exit(0);
  	}
  }

  if (!file || !pathdir || !dimension)
  {
	printf("Usage: %s -d DIMENSION_NAME -s SLICE_NB -l LEVEL -p PATH -f FILE\n", argv[0]);
  	exit(0);
  }
  prctl(PR_SET_NAME, "TS_CORE");

  // initialisation of some variable
  t = malloc(sizeof(*t));
  init_prog(t);
  srand((unsigned)time(NULL));
  // intitialisation the volume

  t->volume_first = NULL;

  // lunch thread_pool
  t->tp = malloc(sizeof(*t->tp));
  thread_pool_init(t->tp, 1);

  // start plugins

  nr_of_plugins = sizeof(PLUGINS) / sizeof(PLUGINS[0]);
  for (x=0;x<nr_of_plugins;x++)
  {
	  sprintf(load_command, "load %s %s/%s", PLUGINS[x][0], PLUGINS_PATH, PLUGINS[x][1]);
	  plugin_load_from_string(load_command, t);
	  DEBUG("Loading: %s\n", load_command);
  }

  int i = 0;
  while (i < level)
  {
  	zoomlevel += 0.25;
  	i++;
  }

  if (level == 0)
    zoomlevel = 0.25;

  if (strcmp(dimension, "xspace") == 0)
  {
  	x_start = slice;
  	x_end = 0;
  }
  else if (strcmp(dimension, "yspace") == 0)
  {
  	y_start = slice;
  	y_end = 0;
  }
  else if (strcmp(dimension, "zspace") == 0)
  {
  	z_start = slice;
  	z_end = 0;
  }

  task_clean_up(t);
  task_lunch(t);

  signal_manager(t);

  asprintf(&tile_command, "start image %s %i %i %i %i %i %i %.4f 1 tiles %s %i -1 -1 grey 0 0 10000 0 0 %s/%i",
	   file, z_start, z_end, y_start, y_end, x_start, x_end, zoomlevel, "png", 256, pathdir, level);

  asprintf(&preview_command, "start image %s %i %i %i %i %i %i %.4f 6 full %s grey 0 0 10000 0 0 %s/%i",
	   file, z_start, z_end, y_start, y_end, x_start, x_end, zoomlevel, "png", pathdir, level);

  char **splitted;
  t_args_plug	*args;


  printf("Creation of preview. Please be patient this could take a while...\n");

  splitted = str_to_wordtab(preview_command);
  args = create_plug_args(splitted, t, NULL);
  plugin_start(args);

  printf("Creation of preview finished\n");

  sleep(2);

  printf("Creation of tiles. Please be patient this could take a while...\n");
  splitted = str_to_wordtab(tile_command);
  args = create_plug_args(splitted, t, NULL);
  plugin_start(args);

  printf("Creation of tiles finished\n");

  printf("Done! Exiting...\n");

  t->tp->loop = 0;
  thread_pool_destroy(t->tp);
  free_core_struct(t);

  return (0);
}
