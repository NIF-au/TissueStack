#include "core.h"

t_plugin	*get_plugin_by_name(char *name, t_plugin *first)
{
  t_plugin	*this;

  // return a pointer on the plugin structure which mach with name
  this = first;
  while (strcmp(name, this->name) != 0 && this != NULL)
    this = this->next;
  if (strcmp(name, this->name) != 0)
    {
      fprintf(stderr, "Plugin name: %s - Not present\n", name);
      return (NULL);
    }
  return (this);
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
    }
  else
    {
      this = a->general_info->first;
      while (this->next != NULL)
	this = this->next;
      this->next = malloc(sizeof(*this->next));
      this = this->next;
    }
  this->name = a->name;
  this->path = a->path;
  this->error = 0;
  this->next = NULL;
  this->busy = 0;
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
  thread_pool_add_task(init, a, a->general_info->tp);
  // run init of the plugin
  //  if ((*init)(a) != 0)
  //  this->error = 2;
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
      this->error = 1;
      return (NULL);
    }
  a->this = this;
  *(void **) (&start) = dlsym(this->handle, "start");
  if ((error = dlerror()) != NULL)
    {
      fprintf(stderr, "%s\n", error);
      dlclose(this->handle);
      this->error = 1;
      return (NULL);
    }
  thread_pool_add_task(start, (void*)a, a->general_info->tp);
  //if ((*start)(a) != 0)
  // this->error = 2;
  //free(a);
  return (NULL);
}

void		*plugin_unload(t_args_plug *a)
{
  t_plugin	*this;
  int		(*close)();
  char		*error;

  if ((this = get_plugin_by_name(a->name, a->general_info->first)) == NULL)
    {
      this->error = 1;
      return (NULL);
    }
  *(void **) (&close) = dlsym(this->handle, "quit");
  if ((error = dlerror()) != NULL)
    {
      fprintf(stderr, "%s\n", error);
      dlclose(this->handle);
      return (NULL);
    }
  // call close function of the plugin
  if ((*close)() != 0)
    this->error = 2;
  free(a);
  return (NULL);
}
