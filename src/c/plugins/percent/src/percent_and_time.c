#include "percent_and_time.h"

void		percent_time_write_plug(char *str, void *box)
{
  printf("\n\npercent = %s\n", str);
  if (str && box)
    fwrite(str, 1, strlen(str), (FILE *)box);
}

t_percent_elem *get_percent_elem_by_id(char *id, t_prcnt_t *p)
{
  t_percent_elem *tmp;

  if (id == NULL || p == NULL || (tmp = p->first_percent) == NULL)
    return (NULL);
  while (tmp != NULL)
    {
      if (strcmp(tmp->id, id) == 0)
	return (tmp);
      tmp = tmp->next;
    }
  return (NULL);
}

void		percent_get(char **commands, void *box, t_prcnt_t *p)
{
  t_percent_elem *tmp;
  char		 pc[30];

  if (p == NULL || p->first_percent == NULL ||
      commands == NULL || commands[1] == NULL)
    return;
  if ((tmp = get_percent_elem_by_id(commands[0], p)) == NULL)
    sprintf(pc, "NULL");
  else
    sprintf(pc, "%f", tmp->percent);
  percent_time_write_plug(pc, box);
}

void		*init(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  LOG_INIT(a);
  INFO("Initialized");
  return (NULL);
}

void		*start(void *args)
{
  t_args_plug	*a;
  t_prcnt_t	*p;

  a = (t_args_plug *)args;
  p = a->general_info->percent;

  DEBUG("Started getting: %s", a->commands[0]);

  percent_get(a->commands, a->box, p);
  return (NULL);
}

void		*unload(void *args)
{
  INFO("Unloaded");
  return (NULL);
}
