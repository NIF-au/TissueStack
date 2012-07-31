void		count_start_init(unsigned long		*start,
				 long unsigned int	*count,
				 t_vol			*volume)
{
  int		i;

  start = malloc(dimensions_nb * sizeof(*start));
  count = malloc(dimensions_nb * sizeof(*count));
  start[0] = start[1] = start[2] = 0;
  i = 0;
  while (i < dimensions_nb)
    {
      count[i] = volume->size[i];
      i++;
    }
}


void		dim_loop(int		fd,
			 int		dimensions_nb,
			 t_vol		*volume)
{
  int		dim = 0;
  int		slice = 0;
  int		this_slice = 0;
  int		size;
  char		*hyperslab;
  unsigned long		start;
  long unsigned int	count;

  count_start_init(&start, &count);
  while (dim < dimensions_nb)
    {
      size = (dim == 0 ? (volume->size[2] * volume->size[1]) :
	      (dim == 1 ? (volume->size[0] * volume->size[2]) : (volume->size[0] * volume->size[1])));
      hyperslab = malloc(size * sizeof(*hyperslab));
      slice = volume->size[dim];
      this_slice = 0;
      &count[dim] = 1;
      while (this_slice < slice)
	{
	  &start[dim] = this_slice;
	  memset(hyperslab, '\0', size);
	  miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE, &start, &count, hyperslab);
	  write(fd, hyperslab, size);
	  printf("Slice = %i - dim = %i\n", this_slice, dim);
	  this_slice++;
	}
      retrive_and_write_one_dimension();
      &start[dim] = 0;
      &count[dim] = volume->size[dim];
      dim++;
      free(hyperslab);
    }
}


int		main(int ac, char **av)
{
  t_tissue_stack	*t;
  int			result;

  int		fd;



  t = malloc(sizeof(*t));
  init_prog(t);
  t->volume_first = malloc(sizeof(*t->volume_first));
  if ((result = init_volume(t->volume_first, argv[1])) != 0)
    return (result);



  fd = open("./raw_data_file", O_CREAT | O_TRUNC | O_RDWR);

  dim_loop(fd, 3, t->volume_first)

  close(fd);
  chmod("./raw_data_file", 0755);

  return (0);
}
