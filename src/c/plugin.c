#include "core.h"

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
