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

t_args_plug	*create_plug_args(char **commands, t_tissue_stack *general)
{
  t_args_plug	*args;
  char		**com;
  t_plugin	*tmp;

  args = malloc(sizeof(*args));
  args->name = malloc((strlen(commands[1]) + 1) * sizeof(*args->path));
  strcpy(args->name, commands[1]);
  args->name = commands[1];
  if (!strcmp(commands[0], "load"))
    {
      args->path = malloc((strlen(commands[2]) + 1) * sizeof(*args->path));
      strcpy(args->path, commands[2]);
      com = copy_args(3, commands);
    }
  else
    {
      tmp = general->first;
      while (tmp)
	{
	  if (!strcmp(tmp->name, args->name))
	    {
	      args->path = malloc((strlen(tmp->name) + 1) * sizeof(*args->path));
	      strcpy(args->path, tmp->name);
	      break;
	    }
	  tmp = tmp->next;
	}
      com = copy_args(2, commands);
    }
  args->commands = com;
  args->general_info = general;
  args->this = NULL;
  return (args);
}

void		prompt_exec(char **commands, t_tissue_stack *general)
{
  int		i;
  t_args_plug	*args;
  char		*prog;
  t_plugin	*p;
  
  
  prog = commands[0];
  args = create_plug_args(commands, general);
  i = 0;
  while (i < general->nb_func)
    {
      if (!strcmp(general->functions[i].name, prog))
	{
	  if (strcmp(prog, "load"))
	    {
	      p = get_plugin_by_name(args->name, general->first);
	      if (p->busy != 0)
		printf("%s: Plugin Busy. Try again later\n", args->name);
	      else
		thread_pool_add_task(general->functions[i].ptr, args, general->tp);
	    }
	  else
	    thread_pool_add_task(general->functions[i].ptr, args, general->tp);
	  break;
	}
      i++;
    }
  if (i == general->nb_func)
    {
      free(args);
      printf("%s: Unknown command\n", prog);
    }
}

void		prompt_start(t_tissue_stack *general)
{
  //  int		flags;
  fd_set	readfd;
  char		buff[4096];
  int		count;
  char		**dest;
  /*
  if ((flags = fcntl(1, F_GETFL, 0)) == -1)
    {
      fprintf(stderr, "Error on fcntl");
      return;
    }
  if (fcntl(1, F_SETFL, flags | O_NONBLOCK) == -1)
    {
      fprintf(stderr, "Error on fcntl");
      return;
      }*/
  FD_ZERO(&readfd);
  FD_SET(0, &readfd);
  while (1)
    {
      pthread_mutex_lock(&(general->tp->lock));
      write(1, "TisseStack > ", 13);
      pthread_mutex_unlock(&(general->tp->lock));
      /*select(1, &readfd, NULL, NULL, NULL);
      if (FD_ISSET(0, &readfd))
      {*/
	  count = read(0, buff, 4096);
	  if (count > 0 && buff[0] != 10)
	    {
	      buff[count - 1] = '\0';
	      if (strcmp(buff, "quit") == 0)
		break;
//      }
	      dest = str_to_wordtab(buff);//prompt_str_to_wordtab(buff);
	      prompt_exec(dest, general);
	    }
    }
}
