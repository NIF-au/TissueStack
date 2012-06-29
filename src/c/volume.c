#include "core.h"

int		init_volume(t_vol *volume, char *path)
{
  int		result;
  int		path_len;

  path_len = strlen(path);
  volume->dim_nb = 3;
  volume->dimensions = malloc(volume->dim_nb * sizeof(*volume->dimensions));
  volume->size = malloc(volume->dim_nb * sizeof(*volume->size));
  volume->path = malloc((path_len + 1) * sizeof(*volume->path));
  volume->path[path_len] = '\0';
  memcpy(volume->path, path, path_len);
  volume->dim_name = malloc(volume->dim_nb * sizeof(*volume->dim_name));
  if (volume->path == NULL)
    return (-1);
  // open the minc file
  if ((result = miopen_volume(volume->path, MI2_OPEN_READ, &volume->minc_volume)) != MI_NOERROR)
    {
      fprintf(stderr, "Error opening input file: %d.\n", result);
      return (-1);
    }
  // get the volume dimensions
  if ((result = miget_volume_dimensions(volume->minc_volume, MI_DIMCLASS_SPATIAL, MI_DIMATTR_ALL,
					MI_DIMORDER_FILE, volume->dim_nb, volume->dimensions)) == MI_ERROR)
    {
      fprintf(stderr, "Error getting dimensions: %d.\n", result);
      return (-1);
    }
  // get the size of each dimensions
  if ((result = miget_dimension_sizes(volume->dimensions, volume->dim_nb, volume->size)) != MI_NOERROR)
    {
      fprintf(stderr, "Error getting dimensiosn size: %d.\n", result);
      return (-1);
    }
  if (miget_dimension_name (volume->dimensions[0], &volume->dim_name[0]) != MI_NOERROR ||
      miget_dimension_name (volume->dimensions[1], &volume->dim_name[1]) != MI_NOERROR ||
      miget_dimension_name (volume->dimensions[2], &volume->dim_name[2]))
    {
      fprintf(stderr, "Error getting dimensiosn name.\n");
      return (-1);
    }
  // get slices_max
  volume->slices_max = get_slices_max(volume);
  volume->next = NULL;
  return (0);
}

void		*file_actions(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  if (strcmp(a->name, "load") == 0)
    add_volume(a->commands[0], a->general_info);
  else if (strcmp(a->name, "unload") == 0)
    remove_volume(a->commands[0], a->general_info);
  return (NULL);
}

void		add_volume(char *path, t_tissue_stack *t)
{
  t_vol		*tmp;
  t_vol		*save;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      save = tmp;
      tmp = tmp->next;
    }
  tmp = malloc(sizeof(*tmp));
  if (init_volume(tmp, path) == 0)
    {
      if (t->volume_first == NULL)
	t->volume_first = tmp;
      else
	save->next = tmp;
    }
  else
    free(tmp);
}

void		list_volumes(t_tissue_stack *t, char *options)
{
  t_vol		*tmp;

  tmp = t->volume_first;
  if (tmp == NULL)
    printf("No volume in the list\n");
  else
    {
      while (tmp != NULL)
	{
	  printf("\nVolume = %s\n", tmp->path);
	  if (options != NULL && strcmp(options, "--verbose") == 0)
	    {
	      printf("\tDimension Number = %i\n", tmp->dim_nb);
	      printf("\tDimension Name = %s - %s - %s\n", tmp->dim_name[0], tmp->dim_name[1], tmp->dim_name[2]);
	      printf("\tDimension sizes = %u - %u - %u\n", tmp->size[0], tmp->size[1], tmp->size[2]);
	    }
	  tmp = tmp->next;
	}
    }
}

t_vol		*get_volume(char *path, t_tissue_stack *t)
{
  t_vol		*tmp;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      if (strcmp(path, tmp->path) == 0)
	return (tmp);
      tmp = tmp->next;
    }
  return (NULL);
}

void		remove_volume(char *path, t_tissue_stack *t)
{
  t_vol		*tmp;
  t_vol		*save;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      if (strcmp(path, tmp->path) == 0)
	{
	  save->next = tmp->next;
	  free_volume(tmp);
	  break;
	}
      save = tmp;
      tmp = tmp->next;
    }
}

void		free_volume(t_vol *v)
{
  int		i;

  i = 0;
  while (i < v->dim_nb)
    {
      mifree_dimension_handle(v->dimensions[i]);
      free(v->dim_name[i]);
      i++;
    }
  free(v->dim_name);
  free(v->dimensions);
  free(v->size);
  free(v->path);
  free(v);
}

void		free_all_volumes(t_tissue_stack *t)
{
  t_vol		*tmp;
  t_vol		*save;

  tmp = t->volume_first;
  while (tmp != NULL)
    {
      miclose_volume(tmp->minc_volume);
      save = tmp;
      tmp = tmp->next;
      free_volume(save);
    }
}
