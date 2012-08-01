#include "core.h"

void		free_all_plugins(t_tissue_stack *t)
{
  t_plugin	*p = NULL;
  t_plugin	*save = NULL;

  p = t->first;
  while (p != NULL)
    {
      save = p;
      dlclose(p->handle);
      if (p->name != NULL) free(p->name);
      if (p->path != NULL) free(p->path);
      p = p->next;
      free(save);
    }
}

void		list_plugins(t_tissue_stack *t, char *command)
{
  t_plugin	*tmp;

  tmp = t->first;
  if (tmp != NULL)
    {
      while (tmp)
	{
	  printf("\nPlugin = %s\n", tmp->name);
	  if (command != NULL && strcmp(command, "--verbose") == 0)
	    {
	      printf("\tPath = %s\n", tmp->path);
	      printf("\tError = %i\n", tmp->error);
	      printf("\tBusy = %s\n", tmp->busy == 0 ? "No" : "Yes");
	    }
	  tmp = tmp->next;
	}
    }
  else
    printf("No plugins loaded\n");
}

void		plug_actions_from_external_plugin(t_tissue_stack *general, char *commands, void *box)
{
  char		**splitted;

  splitted = str_to_wordtab(commands);
  prompt_exec(splitted, general, box);
}

t_plugin	*get_plugin_by_name(char *name, t_plugin *first)
{
  t_plugin	*this;

  if (!name || !first)
    return (NULL);
  // return a pointer on the plugin structure which mach with name
  this = first;
  while (this != NULL && strcmp(name, this->name) != 0)
    this = this->next;
  if (!this || strcmp(name, this->name) != 0)
    return (NULL);
  return (this);
}

void		*plugin_try_start(void *args)
{
  t_args_plug	*a;
  t_args_plug	*a_cpy;

  a = (t_args_plug *)args;
  a->path = strdup(a->commands[0]);
  a_cpy = create_copy_args(a);
  if (!a->general_info->first || get_plugin_by_name(a->name, a->general_info->first) == NULL)
    plugin_load(a);
  a_cpy->this = get_plugin_by_name(a_cpy->name, a->general_info->first);
  a_cpy->commands = &a_cpy->commands[1];
  plugin_start(a_cpy);
  return (NULL);
}

void		*plugin_load(void *args)
{
  t_args_plug	*a;
  void		*(*init)(void *a);
  t_plugin	*this;
  char		*error;

  a = (t_args_plug *)args;
  // loop through the plugin list in order to add a new alement
  if (!a->general_info->first)
    {
      a->general_info->first = malloc(sizeof(*a->general_info->first));
      this = a->general_info->first;
      this->prev = NULL;
    }
  else
    {
      this = a->general_info->first;
      while (this->next != NULL)
	this = this->next;
      this->next = malloc(sizeof(*this->next));
      this->next->prev = this;
      this = this->next;
    }
  this->name = malloc((strlen(a->name) + 1) * sizeof(*this->name));
  this->path = malloc((strlen(a->path) + 1) * sizeof(*this->path));
  this->name = strcpy(this->name, a->name);
  this->path = strcpy(this->path, a->path);
  this->start_command = NULL;
  this->error = 0;
  this->next = NULL;
  this->busy = 0;
  this->thread_id = pthread_self();
  a->this = this;
  // open the plugin
  this->handle = dlopen(this->path, RTLD_LAZY);
  if (!this->handle)
    {
      fprintf(stderr, "%s\n", dlerror());
      this->error = 1;
      return (NULL);
    }
  // clean errors if there was some
  dlerror();
  // get a pointer to the init plugin function
  *(void **) (&init) = dlsym(this->handle, "init");
  if ((error = dlerror()) != NULL)
    {
      fprintf(stderr, "%s\n", error);
      dlclose(this->handle);
      this->error = 1;
      return (NULL);
    }
  // run init of the plugin
  if ((*init)(a) != 0)
    this->error = 2;
  //  free(a);
  return (NULL);
}

void		*plugin_start(void *args)
{
  t_args_plug	*a;
  void		*(*start)(void *a);
  char		*error;
  t_plugin	*this;

  a = (t_args_plug *)args;
  if ((this = get_plugin_by_name(a->name, a->general_info->first)) == NULL)
    {
      if (this)
	this->error = 1;
      return (NULL);
    }
  this->start_command = a->commands;
  a->this = this;
  *(void **) (&start) = dlsym(this->handle, "start");
  if ((error = dlerror()) != NULL)
    {
      fprintf(stderr, "%s\n", error);
      dlclose(this->handle);
      this->error = 1;
      return (NULL);
    }
  //  thread_pool_add_task(start, (void*)a, a->general_info->tp);
  if ((*start)(a) != 0)
    this->error = 2;
  //free(a);
  return (NULL);
}

void		*plugin_unload(void *args)
{
  t_tissue_stack *general;
  t_args_plug	*a;
  t_plugin	*this;
  void		*(*quit)(void *a);
  char		*error;

  a = (t_args_plug *)args;
  general = a->general_info;
  if ((this = get_plugin_by_name(a->name, a->general_info->first)) == NULL)
    {
      this->error = 1;
      return (NULL);
    }
  a->this = this;
  *(void **) (&quit) = dlsym(this->handle, "unload");
  if ((error = dlerror()) != NULL)
    {
      fprintf(stderr, "%s\n", error);
      dlclose(this->handle);
      return (NULL);
    }
  // call close function of the plugin
  if ((*quit)(a) != 0)
    this->error = 2;
  while (this->busy != 0)
    usleep(1000);
  free(this->name);
  free(this->path);
  if (this->prev != NULL)
    this->prev->next = this->next;
  else
    {
      general->first = this->next;
      if (general->first != NULL)
	general->first->prev = NULL;
    }
  free(this);
  return (NULL);
}
