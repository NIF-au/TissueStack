#include "core.h"

int		is_num(char *str)
{
  int		i = 0;

  while (str[i] != '\0')
    {
      if ((str[i] > '9' || str[i] < '0') && str[i] != '.')
	return (0);
      i++;
    }
  return (1);
}

void		percent_time_write(char *str, char **commands, void *box)
{
  int		i = 0;
  char		*tmp;

  while (commands[i + 1] != NULL)
    i++;

  if (commands[i] != NULL && strcmp(commands[i], "file") == 0)
    fwrite(str, 1, strlen(str), (FILE *)box);
  else if (str != NULL && commands[i] != NULL && strcmp(commands[i], "string") == 0)
    {
      tmp = (char *)box;
      i = 0;
      while (str[i] != '\0')
	{
	  tmp[i] = str[i];
	  i++;
	}
      tmp[i] = '\0';
    }
}

void		percent_init_direct(int total_blocks, char **id, t_tissue_stack *t)
{
  t_percent_elem *tmp;
  char		*str_log;
  struct timeval tv;

  gettimeofday(&tv,NULL);
  tmp = malloc(sizeof(*tmp));
  asprintf(&tmp->id, "%i", (int)tv.tv_sec);
  tmp->time = malloc(sizeof(*tmp->time));
  tmp->total_blocks = total_blocks;
  tmp->blocks_done = 0;
  tmp->percent = 0;
  if (!t->percent)
    {
      t->percent = malloc(sizeof(*t->percent));
      t->percent->first_percent = NULL;
    }
  tmp->next = t->percent->first_percent;
  t->percent->first_percent = tmp;

  *id = strdup(tmp->id);

  str_log = tmp->id;
  INFO("Percentage: %s Initialized", str_log);
}

void		percent_init(char **commands, void *box, t_tissue_stack *t)
{
  t_percent_elem *tmp;
  char		*str_log;

  tmp = malloc(sizeof(*tmp));
  tmp->id = malloc(12 * sizeof(*tmp->id));
  tmp->time = malloc(sizeof(*tmp->time));
  tmp->total_blocks = atoi(commands[1]);
  sprintf(tmp->id, "%i", rand() % 1000000000);
  tmp->blocks_done = 0;
  tmp->percent = 0;
  if (!t->percent)
    {
      t->percent = malloc(sizeof(*t->percent));
      t->percent->first_percent = NULL;
    }
  tmp->next = t->percent->first_percent;
  t->percent->first_percent = tmp;

  percent_time_write(tmp->id, commands, box);
  str_log = tmp->id;
  INFO("Percentage: %s Initialized", str_log);
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

void		percent_add_direct(int blocks, char *id, t_tissue_stack *t)
{
  t_percent_elem *tmp;
  char		*str_log;

  if (t->percent == NULL || t->percent->first_percent == NULL ||
      id == NULL)
    return;
  if ((tmp = get_percent_elem_by_id(id, t->percent)) == NULL)
    return;
  tmp->blocks_done += blocks;
  if (tmp->percent < 100)
    tmp->percent = (float)((float)((float)tmp->blocks_done / (float)tmp->total_blocks) * 100.0);
  else
    tmp->percent = 100;
  if (tmp->percent >= 100)
    {
      str_log = tmp->id;
      INFO("Percentage: %s ==> 100%%", str_log);
    }
}

void		percent_add(char **commands, void *box, t_tissue_stack *t)
{
  if (is_num(commands[2]))
    percent_add_direct(atoi(commands[2]), commands[1], t);
}

void		percent_get_direct(char **buff, char *id, t_tissue_stack *t)
{
  t_percent_elem *tmp;

  if (t->percent == NULL || t->percent->first_percent == NULL)
    return;
  if ((tmp = get_percent_elem_by_id(id, t->percent)) == NULL)
    return;
  asprintf(buff, "%f", tmp->percent);
}

void		percent_get(char **commands, void *box, t_tissue_stack *t)
{
  t_percent_elem *tmp;
  char		 pc[30];

  if (t->percent == NULL || t->percent->first_percent == NULL ||
      commands == NULL || commands[1] == NULL || commands[2] == NULL)
    return;
  if ((tmp = get_percent_elem_by_id(commands[1], t->percent)) == NULL)
    return;
  sprintf(pc, "%f", tmp->percent);
  percent_time_write(pc, commands, box);
}

void		percent_destroy(char **commands, void *box, t_tissue_stack *t)
{
  t_percent_elem *tmp;
  t_percent_elem *sav;
  char		*str_log;

  if (t->percent == NULL || (tmp = t->percent->first_percent) == NULL ||
      commands == NULL || commands[1] == NULL || commands[2] == NULL)
    return;
  if (strcmp(t->percent->first_percent->id, commands[1]) == 0)
    ;
  else
    {
      while (tmp != NULL)
	{
	  if (tmp->next != NULL)
	    {
	      if (strcmp(tmp->next->id, commands[1]) == 0)
		break;
	    }
	  tmp = tmp->next;
	}
    }
  if (tmp == NULL)
    return;
  str_log = tmp->next->id;
  INFO("Percentage: %s Destroyed", str_log);
  sav = tmp->next->next;
  free(tmp->next->id);
  free(tmp->next->time);
  free(tmp->next);
  tmp->next = sav;
}

void		percent_time_processing(char **commands, void *box,
					t_func_prcnt_t *f, t_tissue_stack *t)
{
  int		i = 0;

  while (f[i].name != NULL && f[i].func != NULL)
    {
      if (f->name != NULL && strcmp(f->name, commands[0]) == 0)
	f->func(commands, box, t);
      i++;
    }
}

void		init_func_ptr_percent(t_tissue_stack *t)
{
  t->percent->percent_func = malloc(5 * sizeof(*t->percent->percent_func));
  t->percent->percent_func[0].name = strdup("percent_init");
  t->percent->percent_func[1].name = strdup("percent_add");
  t->percent->percent_func[2].name = strdup("percent_get");
  t->percent->percent_func[3].name = strdup("percent_destroy");
  t->percent->percent_func[0].func = percent_init;
  t->percent->percent_func[1].func = percent_add;
  t->percent->percent_func[2].func = percent_get;
  t->percent->percent_func[3].func = percent_destroy;

  t->percent->percent_func[4].name = NULL;
  t->percent->percent_func[4].func = NULL;
}

void		init_func_ptr_time(t_tissue_stack *t)
{
  /*
  p->time_func = malloc(5 * sizeof(*p->time_func));
  p->time_func[0].name = strdup("time_init");
  p->time_func[1].name = strdup("time_add");
  p->time_func[2].name = strdup("time_get");
  p->time_func[3].name = strdup("time_destroy");
  p->time_func[0].func = time_init;
  p->time_func[1].func = time_add;
  p->time_func[2].func = time_get;
  p->time_func[3].func = time_destroy;
  p->time_func[4] = NULL;*/
}

void		init_percent_time(t_tissue_stack *t)
{
  t_prcnt_t	*p;

  p = malloc(sizeof(*p));
  p->first_time = NULL;
  p->first_percent = NULL;
  t->percent = p;
  init_func_ptr_time(t);
  init_func_ptr_percent(t);
  INFO("Percent initialized");
}

/*
void		*init(void *args)
{
  t_args_plug	*a;
  t_prcnt_t	*p;

  a = (t_args_plug *)args;
  INIT_LOG(a);

  p = malloc(sizeof(*p));
  p->first_time = NULL;
  p->first_percent = NULL;
  init_func_ptr_time(p);
  init_func_ptr_percent(p);
  a->general_info->percent = p;
  INFO("Initialized");
  return (NULL);
}

void		*start(void *args)
{
  t_args_plug	*a;
  t_prcnt_t	*p;

  a = (t_args_plug *)args;

  p = a->general_info->percent;
  INFO("Started %s", a->commands[0]);

  if (strncmp(a->commands[0], "percent", 7) == 0)
    percent_time_processing(a->commands, (void *)a->box, p->percent_func, p);
  else if (strncmp(a->commands[0], "time", 4) == 0)
    percent_time_processing(a->commands, (void *)a->box, p->time_func, p);
  return (NULL);
}

void		*unload(void *args)
{
  INFO("Unloaded");
  return (NULL);
}

// "start plug_name percent_init 'total_blocks' (file | string)";
// "start percent_add 'id' 'number_blocks' (file | string)";
// "start percent_get 'id' (file | string)";
// "start percent_destroy 'id'";
*/
