#include "core.h"

void		*test_load(t_args_plug *a)
{
  printf("loader func\n");
  return (NULL);
}

void		*test_start(t_args_plug *a)
{
  printf("start\n");
  return (NULL);
}

unsigned int            get_slices_max(t_vol *volume)
{
  // get the larger number of slices possible
  if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[X]))
    {
      if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[Y]))
        return (volume->size[X] * volume->size[Y]);
    }
  else if ((volume->size[Z] * volume->size[X]) > (volume->size[Z] * volume->size[Y]))
    return (volume->size[Z] * volume->size[X]);
  return (volume->size[Z] * volume->size[Y]);
}

void		init_func_ptr(t_tissue_stack *t)
{
  t_function	*functions;

  functions = malloc(4 * sizeof(*functions));
  functions[0].name = "load";
  functions[1].name = "start";
  //  functions[2].name = "unload";
  functions[0].ptr = plugin_load;
  functions[1].ptr = plugin_start;
  // functions[2].ptr = plugin_unload;
  t->functions = functions;
  t->nb_func = 2;
}

void            init_prog(t_tissue_stack *t, char *path_arg, int nb_args)
{
  t->volume = malloc(sizeof(*t->volume));
  // set the number of dimensions of the volume
  t->volume->dim_nb = 3;
  /* Allocation of all the volume component needed (dimensions, sizes and path of the file)
  ** Initialisation of some variables */
  t->volume->dimensions = malloc(t->volume->dim_nb * sizeof(*t->volume->dimensions));
  t->volume->size = malloc(t->volume->dim_nb * sizeof(*t->volume->size));
  if (nb_args == 2)
    {
      t->volume->path = malloc((strlen(path_arg) + 1) * sizeof(*t->volume->path));
      memcpy(t->volume->path, path_arg, strlen(path_arg));
    }
  else
    t->volume->path = NULL;
  t->first = NULL;
  init_func_ptr(t);
}

int		init_volume(t_vol *volume)
{
  int		result;

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
  // get slices_max
  volume->slices_max = get_slices_max(volume);
  return (0);
}

int		main(int argc, char **argv)
{
  int			result;
  t_tissue_stack	*t;

  // initialisation of some variable
  t = malloc(sizeof(*t));
  init_prog(t, argv[1], argc);
  // intitialisation the volume
  if (argc > 1)
    {
      if ((result = init_volume(t->volume)) != 0)
	return (result);
    }
  
  // lunch thread_pool
  t->tp = malloc(sizeof(*t->tp));
  thread_pool_init(t->tp, 6);

  // lunch the prompt command
  prompt_start(t);
  
  // free all the stuff mallocked  
  thread_pool_destroy(t->tp);

  /*
  free(volume.dimensions);
  free(volume.size);
  free(volume.path);
  */
  return (0);
}
