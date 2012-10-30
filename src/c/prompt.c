#include "core.h"

int		word_count(char *buff)
{
  int		i;
  int		count;

  i = 0;
  count = 0;
  if (buff[0] != ' ' && buff[0] != '\t' && buff[0] != '\0')
    count++;
  while (buff[i] != '\0')
    {
      if (((buff[i] == ' ' && buff[i + 1] != ' ' ) ||
	   (buff[i] == '\t' && buff[i + 1] != '\t' )) && buff[i + 1] != '\0')
	count++;
      i++;
    }
  return (count);
}

int		letter_count(char *buff, int position)
{
  int		i;

  i = 0;
  while (buff[position + i] != ' ' && buff[position + i] != '\0' && buff[position + i] != '\t')
    i++;
  return (i);
}

char		*str_n_cpy(char *str, int position, int len)
{
  char		*dest;
  int		i;

  i = 0;
  dest = malloc((len + 1) * sizeof(*dest));
  while (i < len)
    {
      dest[i] = str[i + position];
      i++;
    }
  dest[i] = '\0';
  return (dest);
}

char		**str_to_wordtab(char *buff)
{
  int		i;
  int		j;
  char		**dest;

  dest = malloc((word_count(buff) + 1) * sizeof(*dest));
  i = 0;
  j = 0;
  while (buff[i] != '\0')
    {
      if (buff[i] != ' ' && buff[i] != '\t' && buff[i] != '\0')
	{
	  dest[j] = str_n_cpy(buff, i, letter_count(buff, i));
	  j++;
	  i += letter_count(buff, i);
	}
      if (buff[i] != '\0')
	i++;
    }
  dest[j] = NULL;
  return (dest);
}

char		**copy_args(int start, char **commands)
{
  int		i;
  char		**dest;

  i = 0;
  while (commands[i] != NULL)
    i++;
  dest = malloc(((i - start) + 2) * sizeof(*dest));
  i = 0;
  while (commands[i + start] != NULL)
    {
      dest[i] = malloc((strlen(commands[i + start]) + 1) * sizeof(*dest[i]));
      strcpy(dest[i], commands[i + start]);
      i++;
    }
  dest[i] = NULL;
  return (dest);
}

void		destroy_plug_args(t_args_plug *a)
{
  if (a == NULL) {
	  return;
  }

  destroy_command_args(a->commands);

  if (a->name != NULL) {
    free(a->name);
    a->name = NULL;
  }
  if (a->path != NULL) {
	  free(a->path);
	  a->path = NULL;
  }

  free(a);
  a = NULL;
}

void		destroy_command_args(char ** args)
{
  if (args == NULL) {
	  return;
  }

  int i=0;
  while (args[i] != NULL) {
	  if (args[i] != NULL) free(args[i]);
	  args[i] = NULL;
	  i++;
  }

  free(args);
  args = NULL;
}

t_args_plug	*create_copy_args(t_args_plug *args)
{
  t_args_plug	*a;

  if (args != NULL)
  a = malloc(sizeof(*a));
  a->name = strdup(args->name);
  a->path = strdup(args->path);
  a->commands = copy_args(0, args->commands);
  a->general_info = args->general_info;
  a->this = (args->this != NULL ? args->this : NULL);
  a->box = args->box;
  a->destroy = destroy_plug_args;
  return (a);
}

t_args_plug	*create_plug_args(char **commands, t_tissue_stack *general, void *box)
{
  t_args_plug	*args;
  char		**com;
  t_plugin	*tmp;

  args = malloc(sizeof(*args));
  if (commands[1] == NULL)
    return (NULL);
  args->name = strdup(commands[1]);
  if (!strcmp(commands[0], "load"))
    {
      args->path = strdup(commands[2]);
      com = copy_args(3, commands);
    }
  else
    {
      tmp = general->first;
      while (tmp)
	{
	  if (!strcmp(tmp->name, args->name))
	    {
	      args->path = strdup(tmp->name);
	      break;
	    }
	  tmp = tmp->next;
	}
      com = copy_args(2, commands);
    }

  args->commands = com;
  args->general_info = general;
  args->this = NULL;
  args->box = box;
  args->destroy = destroy_plug_args;
  return (args);
}

void		prompt_exec(char **commands, t_tissue_stack *general, void *box)
{
  int		i = 0;
  t_args_plug	*args;
  char		*prog;
  t_plugin	*p;


  prog = commands[0];
  if (strcmp(commands[0], "resume") == 0)
    {
      general->percent_resume(commands[1], general);
      return;
    }

  args = create_plug_args(commands, general, box);
  while (i < general->nb_func)
    {
      if (strcmp(general->functions[i].name, prog) == 0)
	{
	  p = get_plugin_by_name(args->name, general->first);
	  if (p && p->busy != 0) {
	    printf("%s: Plugin busy. We wait for a bit...\n", args->name);
	    int waitLoops = 0;
	    while (p && p->busy != 0 && waitLoops < 5) {
	      usleep(100000);
	      waitLoops++;
	    }
	    if (p && p->busy != 0) printf("%s: Plugin TOO Busy. Try again later\n", args->name);
	  }
	  else if (!p && (strcmp(prog, "load") != 0 || strcmp(prog, "try_start") != 0) &&
		   (strcmp(prog, "start") == 0 || strcmp(prog, "unload") == 0))
	    {
	      ERROR("%s - %p: Unknown Plugin", args->name, p);
	      if (box)
		fclose((FILE*)box);
	    }
	  else
	    thread_pool_add_task(general->functions[i].ptr, args, general->tp);
	  break;
	}
      i++;
    }

  destroy_command_args(commands);

  if (i == general->nb_func)
    {
      destroy_plug_args(args);
      printf("%s: Unknown command\n", prog);
    }
}

void		change_tty_attr()
{
  static int	flag = 0;
  struct termios current;
  static struct termios save;

  if (flag == 0)
    {
      if (tcgetattr(0, &current) == -1)
	{
	  perror("ioctl");
	  write(2, "Error ioctl get\n", strlen("Error ioctl get\n"));
	  return;
	}
      if (tcgetattr(0, &save) == -1)
	{
	  perror("ioctl");
	  write(2, "Error ioctl get\n", strlen("Error ioctl get\n"));
	  return;
	}
      current.c_lflag &= ~(ICANON | ECHO);
      current.c_cc[VMIN] = 1;
      current.c_cc[VTIME] = 0;
      tcflush(0, TCIFLUSH);
      if (tcsetattr(0, TCSANOW, &current)== -1)
	{
	  perror("tcset");
	  write(2, "Error ioctl set\n", strlen("Error ioctl set\n"));
	  return;
	}
      flag++;
    }
  else
    {
      if (tcsetattr(0, TCSANOW, &save)== -1)
	{
	  perror("tcset");
	  write(2, "Error ioctl set\n", strlen("Error ioctl set\n"));
	  return;
	}
    }
}

t_char_prompt	*get_current_position_prompt(t_tissue_stack *general)
{
  t_char_prompt *c;

  c = general->prompt_first;
  if (c != NULL)
    {
      while (!c->position)
	c = c->next;
      return (c);
    }
  return (NULL);
}

void		aff_prompt(t_tissue_stack *general)
{
  t_char_prompt	*c;
  int		i;

  write(1, "\r\033[2K", strlen("\r\033[2K"));
  write(1, "TisseStack > ", 13);
  c = general->prompt_first;
  while (c && c->next != general->prompt_first)
    {
      write(1, &c->c, 1);
      c = c->next;
    }
  if (c)
    write(1, &c->c, 1);
  if (general->prompt_first)
    {
      c = general->prompt_first->prev;
      i = 0;
      if (c != general->prompt_first)
	{
	  while (!c->position)
	    {
	      c = c->prev;
	      write(1, "\033[1D", strlen("\033[1D"));
	      i++;
	    }
	}
    }
}

int		get_position_nb(t_tissue_stack *general)
{
  t_char_prompt *c;
  int		i;

  i = 0;
  c = general->prompt_first;
  if (c != NULL)
    {
      while (!c->position)
	{
	  c = c->next;
	  i++;
	}
    }
  return (i);
}

void		delete_node(t_tissue_stack *general)
{
  t_char_prompt	*c;
  t_char_prompt	*tmp;

  c = general->prompt_first;
  if (c)
    {
      if (c->position)
	{
	  tmp = c->next;
	  if (c->next != c && c->prev != c)
	    {
	      c->next->prev = c->prev;
	      c->prev->next = c->next;
	      free(c);
	      general->prompt_first = tmp;
	      general->prompt_first->position = 1;
	    }
	  else
	    general->prompt_first = NULL;
	}
      else
	{
	  c = c->next;
	  while (c != general->prompt_first)
	    {
	      if (c->position)
		{
		  tmp = c->prev;
		  c->next->prev = c->prev;
		  c->prev->next = c->next;
		  tmp->position = 1;
		  free(c);
		  break;
		}
	      c = c->next;
	    }
	}
    }
}

void		prompt_backspace(t_tissue_stack *general)
{
  delete_node(general);
  aff_prompt(general);
}

void		clear_prompt_list(t_tissue_stack *general)
{
  t_char_prompt *c;
  t_char_prompt *tmp;
  t_char_prompt *tmp2;

  c = general->prompt_first;
  tmp = c->next;
  if (tmp != c)
    {
      while (tmp != c)
	{
	  tmp2 = tmp;
	  tmp = tmp->next;
	  free(tmp2);
	}
    }
  free(tmp);
  general->prompt_first = NULL;
}

int		prompt_len(t_tissue_stack *general)
{
  t_char_prompt	*c;
  int		i;

  i = 1;
  c = general->prompt_first->next;
  while (c != general->prompt_first)
    {
      i++;
      c = c->next;
    }
  return (i);
}

void		add_history(t_tissue_stack *general)
{
  int		i;
  char		*str;
  t_hist_prompt	*tmp;
  t_hist_prompt	*tmp2;
  t_char_prompt	*c;

  str = malloc((prompt_len(general) + 1) * sizeof(*str));
  i = 1;
  str[0] = general->prompt_first->c;
  c = general->prompt_first->next;
  while (c != general->prompt_first)
    {
      str[i++] = c->c;
      c = c->next;
    }
  str[i] = '\0';
  if (general->hist_first == NULL)
    {
      tmp = malloc(sizeof(*tmp));
      tmp->prev = tmp;
      tmp->next = tmp;
      tmp->position = 1;
      tmp->commands = str;
      general->hist_first = tmp;
    }
  else
    {
      tmp = general->hist_first->prev;
      tmp2 = malloc(sizeof(*tmp2));
      tmp2->commands = str;
      tmp2->position = 1;
      tmp2->prev = tmp;
      tmp2->next = general->hist_first;
      tmp->next = tmp2;
      tmp->position = 0;
      general->hist_first->prev = tmp2;
    }
}

void		prompt_enter(t_tissue_stack *general)
{
  t_char_prompt *c;
  int		i;
  int		j;
  char		*dest;
  char		**commands;

  if (general->prompt_first == NULL)
    return;
  c = general->prompt_first;
  add_history(general);
  dest = malloc((prompt_len(general) + 1) * sizeof(*dest));
  j = 0;
  i = prompt_len(general);
  while (j < i)
    {
      dest[j] = c->c;
      c = c->next;
      j++;
    }
  dest[j] = '\0';
  write(1, "\n", 1);
  commands = str_to_wordtab(dest);
  if (!strcmp(commands[0], "quit"))
    {
      general->quit = 1;
      i = 0;
      while (commands[i] != NULL)
	free(commands[i++]);
      free(commands);
      free(dest);
    }
  else
    {
      prompt_exec(commands, general, NULL);
      clear_prompt_list(general);
      write(1, "TissueStack > ", 13);
      free(dest);
    }
}

void		prompt_left(t_tissue_stack *general)
{
  t_char_prompt *c;

  c = general->prompt_first;
  if (c)
    {
      while (!c->position)
	c = c->next;
      if (c != general->prompt_first)
	{
	  c->prev->position = 1;
	  if (c->prev != c)
	    {
	      write(1, "\033[1D", strlen("\033[1D"));
	      c->position = 0;
	    }
	}
    }
}

void		prompt_right(t_tissue_stack *general)
{
  t_char_prompt *c;

  c = general->prompt_first;
  if (c)
    {
      while (!c->position)
	c = c->next;
      if (c->next != general->prompt_first)
	{
	  c->next->position = 1;
	  if (c->next != c)
	    {
	      write(1, "\033[1C", strlen("\033[1C"));
	      c->position = 0;
	    }
	}
    }
}

void		get_from_history(t_tissue_stack *general, int key)
{
  t_hist_prompt	*tmp;
  t_char_prompt	*c;
  t_char_prompt	*c_first;
  int		i;

  i = 0;
  tmp = general->hist_first;
  if (general->hist_first == NULL)
    return;
  while (tmp->position == 0)
    tmp = tmp->next;
  c_first = malloc(sizeof(*c_first));
  c = c_first;
  if (key == -1)
    {
      tmp->position = 0;
      tmp->prev->position = 1;
    }
  else
    {
      tmp->position = 0;
      tmp->next->position = 1;
    }
  while (tmp->commands[i] != '\0')
    {
      c->c = tmp->commands[i];
      c->position = 0;
      i++;
      if (tmp->commands[i] != '\0')
	{
	  c->next = malloc(sizeof(*c->next));
	  c->next->prev = c;
	  c = c->next;
	}
    }
  c->position = 1;
  c_first->prev = c;
  c->next = c_first;
  general->prompt_first = c_first;
}

void		prompt_up(t_tissue_stack *general)
{
  get_from_history(general, -1);
  aff_prompt(general);
}

void		prompt_down(t_tissue_stack *general)
{
  get_from_history(general, 1);
  aff_prompt(general);
}

int		action_keys(char *buff, t_tissue_stack *general)
{
  if (buff[0] < 32 || buff[0] > 126)
    {

      if (buff[0] == 27 && buff[1] == 91 && buff[2] == 65 && buff[3] == 0)
	prompt_up(general);
      else if (buff[0] == 27 && buff[1] == 91 && buff[2] == 66 && buff[3] == 0)
	prompt_down(general);
      else if (buff[0] == 27 && buff[1] == 91 && buff[2] == 67 && buff[3] == 0)
	prompt_right(general);
      else if (buff[0] == 27 && buff[1] == 91 && buff[2] == 68 && buff[3] == 0)
	prompt_left(general);
      /*else if (buff[0] == 27 && buff[1] == 0)
	printf("echap\n");
      else if (buff[0] == 9 && buff[1] == 0)
      printf("tab\n");*/
      else if (buff[0] == 127 && buff[1] == 0)
	prompt_backspace(general);
      else if (buff[0] == 10 && buff[1] == 0)
	prompt_enter(general);
      return (1);
    }
  return (0);
}

void		stringify_char(char buff, t_tissue_stack *general)
{
  t_char_prompt	*c;
  t_char_prompt	*tmp;

  if (general->prompt_first == NULL)
    {
      general->prompt_first = malloc(sizeof(*general->prompt_first));
      c = general->prompt_first;
      c->prev = c;
      c->next = c;
      c->c = buff;
      c->position = 1;
    }
  else
    {
      c = general->prompt_first;
      if (!c->position)
	{
	  c = c->next;
	  while (c != general->prompt_first)
	    {
	      if (c->position)
		break;
	      c = c->next;
	    }
	}
      tmp = malloc(sizeof(*tmp));
      tmp->c = buff;
      tmp->position = 1;
      tmp->prev = c;
      tmp->next = c->next;
      c->next->prev = tmp;
      c->next = tmp;
      c->position = 0;
      c = general->prompt_first;
      while (c->next != general->prompt_first)
	c = c->next;
      general->prompt_first->prev = c;
    }
}

void		free_all_prompt(t_tissue_stack *t)
{
  t_char_prompt	*c = NULL;
  t_char_prompt	*save = NULL;

  c = t->prompt_first;
  if (c != NULL && c->next == t->prompt_first)
    {
      free(t->prompt_first);
    }
  else if (c != NULL)
    {
      c = c->next;
      while (c != t->prompt_first)
	{
	  save = c;
	  c = c->next;
	  free(save);
	}
      free(t->prompt_first);
    }
}

void		free_all_history(t_tissue_stack *t)
{
  t_hist_prompt	*c = NULL;
  t_hist_prompt	*save = NULL;

  c = t->hist_first;
  if (c != NULL && c->next == t->hist_first)
    {
      free(t->hist_first->commands);
      free(t->hist_first);
    }
  else if (c != NULL)
    {
      c = c->next;
      while (c != t->hist_first)
	{
	  save = c;
	  c = c->next;
	  free(c->commands);
	  free(save);
	}
      free(t->hist_first);
    }
}

void		prompt_start(t_tissue_stack *general)
{
  char		buff[4096];
  int		count;

  general->quit = 0;
  general->prompt_first = NULL;
  general->hist_first = NULL;
  change_tty_attr();
  write(1, "TissueStack > ", 13);
  while (general->quit == 0)
    {
      memset(buff, '\0', 4096);
      count = read(0, buff, 4096);
      if (count > 0)
	{
	  buff[count] = '\0';
	  if (!action_keys(buff, general))
	    {
	      stringify_char(buff[0], general);
	      aff_prompt(general);
	    }
	}
    }
  change_tty_attr();
}
