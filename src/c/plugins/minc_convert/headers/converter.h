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
#ifndef __MINC_TOOL_CORE__
#define __MINC_TOOL_CORE__

#include "core.h"
#include "utils.h"

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
void		dim_loop(int fd, int dimensions_nb, t_vol *volume,
			 t_tissue_stack *t, char *id_percent,
			 int slice_resume, int dimension_resume);

t_vol			*init_get_volume_from_minc_file(char *path);
t_header		*create_header_from_minc_struct(t_vol *minc_volume);
void			write_header_into_file(int fd, t_header *h);
void 			turn_into_generic_raw(char * hyperslab);

extern  t_log_plugin log_plugin;

#endif /* __MINC_TOOL_CORE__ */
