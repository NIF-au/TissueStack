#ifndef __MINC_TOOL_CORE__
#define __MINC_TOOL_CORE__

#include "core.h"

#include <sys/stat.h>

typedef struct	s_header	t_header;

struct			s_header
{
  int			dim_nb;
  unsigned int		*sizes;
  double		*start;
  double		*steps;
  char			**dim_name;
  unsigned long long int	*dim_offset;
  unsigned int		*slice_size;
  unsigned int		slice_max;
};

unsigned int            get_slices_max(t_vol *volume);
void			count_start_init(unsigned long		*start,
					 long unsigned int	*count,
					 t_vol			*volume);
void			dim_loop(int		fd,
				 int		dimensions_nb,
				 t_vol		*volume,
				 t_tissue_stack	*t,
				 char		*id_percent);
t_vol			*init_get_volume_from_minc_file(char *path);
t_header		*create_header_from_minc_struct(t_vol *minc_volume);
void			write_header_into_file(int fd, t_header *h);

#endif /* __MINC_TOOL_CORE__ */
