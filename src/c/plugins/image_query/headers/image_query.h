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
#ifndef __IMAGE_QUERY__
#define __IMAGE_QUERY__

#include "core.h"
#include "utils.h"

#include <minc2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <math.h>

#include <magick/api.h>

typedef	struct	s_image_extract
{
  int 		dim_nb;
  int		**dim_start_end;
  unsigned int	total_slices_to_do;
  unsigned int	slices_done;
  float		percent;
  unsigned int	step;
  float		scale;
  int		quality;
  int		start_h;
  int		start_w;
  int		h_position;
  int		w_position;
  int		h_position_end;
  int		w_position_end;
  char		*service;
  int		square_size;
  char 		*image_type;
  int		done;
  char		*root_path;
  float		***premapped_colormap;
  int		colormap_id;
  char		**colormap_name;
  unsigned char	contrast_min;
  unsigned char	contrast_max;
  int		contrast;
  char 		*request_id;
  char 		*request_time;
  char		*id_percent;
  int		percentage;
  int		percent_fd;
  pthread_cond_t	cond;
  pthread_mutex_t	mut;
  pthread_mutex_t	percent_mut;
}		t_image_extract;

typedef struct  s_image_args
{
  int			**dim_start_end;
  t_vol			*volume;
  t_thread_pool		*p;
  t_plugin		*this;
  t_image_extract	*info;
  FILE			*file;
  t_tissue_stack	*general_info;
}               t_image_args;

#define X 0
#define Y 1
#define Z 2

void            *init(void *args);
void            *start(void *args);
void			*unload(void *args);

void			free_image_extract(t_image_extract * extract);
void			free_image_args(t_image_args * args);

extern  t_log_plugin	log_plugin;

#endif		/* __IMAGE_QUERY__ */
