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
      //if (p->stock != NULL) free(p->stock);
      p = p->next;
      free(save);
      save = NULL;
    }
}

void		list_plugins(t_tissue_stack *t, char *command)
{
  t_plugin	*tmp = NULL;

  tmp = t->first;
  if (tmp != NULL)
    {
      while (tmp)
	{
	  printf("\nPlugin = %s\n", tmp->name);
	  if (command != NULL && strcmp(command, "--verbose") == 0)
	    {
	      printf("\tAdress = %p\n", tmp);
	      printf("\tid = %i", tmp->id);
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

t_plugin	*get_plugin_by_id(int id, t_tissue_stack *t)
{
  t_plugin	*p;

  if (!t)
    return (NULL);
  if ((p = t->first) == NULL)
    return (NULL);
  while (p != NULL)
    {
      if (p->id == id)
	return (p);
      p = p->next;
    }
  return (NULL);
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

void		plugin_load_from_string(char *str, t_tissue_stack *t)
{
  t_args_plug	*args;

  args = create_plug_args(str_to_wordtab(str), t, NULL);
  plugin_load(args);
}

t_plugin	*plugindup(t_plugin *p)
{
  t_plugin	*new;
  int		i;

  if (p == NULL)
    return (NULL);
  new = malloc(sizeof(*new));
  new->error = p->error;
  new->busy = p->busy;
  new->id = p->id;
  i = 0;
  if (p->start_command)
    {
      while (p->start_command[i] != NULL)
	i++;
      new->start_command = malloc((i + 1) * sizeof(*new->start_command));
      i = 0;
      while (p->start_command[i] != NULL)
	{
	  if (p->start_command[i])
	    new->start_command[i] = NULL;//strdup(p->start_command[i]);
	  i++;
	}
      new->start_command[i] = NULL;
    }
  else
    new->start_command = NULL;
  new->name = strdup(p->name);
  new->path = strdup(p->path);
  new->handle = NULL;
  new->stock = NULL;
  new->prev = NULL;
  new->next = NULL;
  return (new);
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
  this->id = rand();
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
  return (NULL);
}

void		plugin_start_from_string(char *str, t_tissue_stack *t)
{
  t_args_plug	*args;

  args = create_plug_args(str_to_wordtab(str), t, NULL);
  plugin_start(args);
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

  destroy_t_plugin(this, general);
  destroy_plug_args(a);

  return (NULL);
}

void destroy_t_plugin(t_plugin * this, t_tissue_stack * general)
{
  if (this == NULL) return;

  if (this->prev != NULL)
    this->prev->next = this->next;
  else
    {
      general->first = this->next;
      if (general->first != NULL) general->first->prev = NULL;
    }

  if (this->start_command != NULL) destroy_command_args(this->start_command);
  if (this->handle != NULL) dlclose(this->handle);
  if (this->name != NULL) free(this->name);
  if (this->path != NULL) free(this->path);
  if (this->stock != NULL) free(this->stock);

  free(this);
}
