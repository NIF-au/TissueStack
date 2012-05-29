/*
** core.c for hello in /home/oliver/workspace/TissueStack/src/c
**
** Made by Oliver Nicolini
** E-Mail   o.nicolini@uq.edu.au
**
** Started on  Mon May 21 13:05:15 2012 Oliver Nicolini
** Last update Tue May 29 14:47:05 2012 Oliver Nicolini
*/


#include "core.h"

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

  functions = malloc(5 * sizeof(*functions));
  functions[0].name = "load";
  functions[1].name = "start";
  functions[2].name = "unload";
  functions[3].name = "file";
  functions[4].name = "list";
  functions[0].ptr = plugin_load;
  functions[1].ptr = plugin_start;
  functions[2].ptr = plugin_unload;
  functions[3].ptr = file_actions;
  functions[4].ptr = list_actions;
  t->functions = functions;
  t->nb_func = 5;
}

void            init_prog(t_tissue_stack *t)
{
  t->plug_actions = plug_actions_from_external_plugin;
  t->get_volume = get_volume;
  t->first = NULL;
  init_func_ptr(t);
}

int		main(int argc, char **argv)
{
  int			result;
  t_tissue_stack	*t;

  // initialisation of some variable
  t = malloc(sizeof(*t));
  init_prog(t);
  // intitialisation the volume
  if (argc > 1)
    {
      t->volume_first = malloc(sizeof(*t->volume_first));
      if ((result = init_volume(t->volume_first, argv[1])) != 0)
	return (result);
    }
  else
    t->volume_first = NULL;
  // lunch thread_pool
  t->tp = malloc(sizeof(*t->tp));
  thread_pool_init(t->tp, 6);

  (t->plug_actions)(t, "load png ./plugins/png_extract/yop.so", NULL);
  sleep(1);
  (t->plug_actions)(t, "load serv ./plugins/communicator/serv.so", NULL);
  sleep(1);
  (t->plug_actions)(t, "start png /home/oliver/workspace/brain.mnc 80 81 -1 -1 -1 -1 1", NULL);
  //  (t->plug_actions)(t, "start serv 4242", NULL);
  // lunch the prompt command
  prompt_start(t);

  // free all the stuff mallocked
  thread_pool_destroy(t->tp);

  return (0);
}
