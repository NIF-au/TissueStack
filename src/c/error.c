#include "core.h"

void		add_error(t_tissue_stack  *general, int signal, t_plugin *plug)
{
  t_error	*tmp;
  time_t	now;

  if (general->first_error == NULL)
    {
      general->first_error = malloc(sizeof(*general->first_error));
      general->first_error->next = NULL;
    }
  tmp = general->first_error;
  while (tmp->next == NULL)
    tmp = tmp->next;
  tmp->next = malloc(sizeof(*tmp->next));
  tmp = tmp->next;
  tmp->signal = signal;
  tmp->plugin = plug;
  tmp->id = rand();
  now = time(0);
  tmp->time = localtime(&now);
}

void		remove_error_by_id(t_tissue_stack *general, int id)
{
  t_error	*tmp;
  t_error	*save;

  if (general->first_error == NULL)
    {
      fprintf(stderr, "Error on Error management : remove error\n");
      return;
    }
  tmp = general->first_error;
  if (tmp->id == id)
    {
      save = tmp->next;
      free(tmp);
      general->first_error = save;
      return;
    }
  while (tmp != NULL)
    {
      save = tmp;
      tmp = tmp->next;
      if (tmp != NULL && tmp->id == id)
	{
	  save->next = tmp->next;
	  free(tmp);
	  break;
	}
    }
}

int		get_errors_nb_by_plugin(t_tissue_stack *general, t_plugin * plug)
{
  int		error_nb;
  t_error	*tmp;

  error_nb = 0;
  if (general->first_error == NULL)
    return (0);
  tmp = general->first_error;
  while (tmp != NULL)
    {
      if (tmp->plugin && tmp->plugin == plug)
	error_nb++;
      tmp = tmp->next;
    }
  return (error_nb);
}

int		*get_error_by_plugin(t_tissue_stack *general, t_plugin *plug)
{
  t_error	*tmp;
  int		*errors;
  int		error_nb;
  int		i;

  error_nb = 0;
  if (general->first_error == NULL)
    return (NULL);
  tmp = general->first_error;
  while (tmp != NULL)
    {
      if (tmp->plugin && tmp->plugin == plug)
	error_nb++;
      tmp = tmp->next;
    }
  if (error_nb == 0)
    return (NULL);
  errors = malloc((error_nb + 1) * sizeof(*errors));
  tmp = general->first_error;
  i = 0;
  while (tmp != NULL)
    {
      if (tmp->plugin && tmp->plugin == plug)
	{
	  errors[i] = tmp->signal;
	  i++;
	}
      tmp = tmp->next;
    }
  errors[i] = -1;
  return (errors);
}

void		clean_error_list(t_tissue_stack *general, int min)
{
  t_error	*tmp;
  struct tm	*now_ptr;
  time_t	now;

  if (general->first_error == NULL)
    return;
  now = time(0);
  now_ptr = localtime(&now);
  tmp = general->first_error;
  while (tmp != NULL)
    {
      if (now_ptr->tm_min < (tmp->time->tm_min + min))
	remove_error_by_id(general, tmp->id);
      tmp = tmp->next;
    }
}
