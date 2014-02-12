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

unsigned int            get_slices_max(t_vol *volume);
void			count_start_init(unsigned long		*start,
					 long unsigned int	*count,
					 t_vol			*volume);
void		dim_loop(int fd, int dimensions_nb, t_vol *volume,
			 t_tissue_stack *t, char *id_percent,
			 int slice_resume, int dimension_resume);

t_vol			*init_get_volume_from_minc_file(char *path);

t_header	*create_header_from_minc_struct(t_vol *minc_volume)
{
  t_header	*h;
  int		i;
  int		j;

  h = malloc(sizeof(*h));

  h->dim_nb = minc_volume->dim_nb;

  h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
  h->start = malloc(h->dim_nb * sizeof(*h->start));
  h->steps = malloc(h->dim_nb * sizeof(*h->steps));
  h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
  h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
  h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));

  h->slice_max = minc_volume->slices_max;

  i = 0;
  while (i < h->dim_nb)
    {
      h->sizes[i] = minc_volume->size[i];
      h->start[i] = minc_volume->starts[i];
      h->steps[i] = minc_volume->steps[i];
      h->dim_name[i] = strdup(minc_volume->dim_name[i]);

      h->slice_size[i] = 1;
      j = 0;
      while (j < h->dim_nb)
	{
	  if (j != i)
	    h->slice_size[i] *= minc_volume->size[j];
	  j++;
	}
      i++;
    }

  h->dim_offset[0] = 0;
  i = 1;
  while (i < h->dim_nb)
    {
      h->dim_offset[i] = (unsigned long long)(h->dim_offset[i - 1] + (unsigned long long)((unsigned long long)h->slice_size[i - 1] * (unsigned long long)h->sizes[i - 1]));
      i++;
    }
  return (h);
}

extern  t_log_plugin log_plugin;

#endif /* __MINC_TOOL_CORE__ */
