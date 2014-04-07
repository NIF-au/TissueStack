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
#ifndef __IMAGE_EXTRACT__
#define __IMAGE_EXTRACT__

#include "core.h"
#include "utils.h"

#include <minc2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <math.h>

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

unsigned int    get_total_slices_to_do(t_vol *v, int **dim_start_end);
int             **generate_dims_start_end(t_vol *v, int sx, int ex, int sy,
                                          int ey, int sz, int ez);
int             **generate_dims_start_end_thread(t_vol *v, int dim, int start, int end);
t_image_extract	*create_image_struct();
t_image_args      *create_args_thread(t_tissue_stack *t, t_vol *vol, t_image_extract *image_general, FILE *sock);
void            image_creation_lunch(t_tissue_stack *t, t_vol *vol, int step, t_image_extract *image_general, FILE *sock);
void			lunch_percent_display(t_tissue_stack *t, t_vol *vol, t_image_extract *image_general);
void            *init(void *args);
void            *start(void *args);

unsigned int            get_slices_max(t_vol *volume);
int		get_nb_blocks_percent(t_image_extract *img, t_vol *volume);
void            *get_all_slices_of_all_dimensions(void *args);
void            get_all_slices_of_one_dimension(t_vol *volume, unsigned long *start, int current_dimension,
                                                long unsigned int *count, t_image_args *a);

unsigned char 	*get_raw_data(t_memory_mapping * memory_mappings, t_vol *volume, int dim, int slice, short * free_data);

void            set_grayscale(unsigned char *ptr, float val);
void            print_image(unsigned char *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_image_args *a);

void			free_image_extract(t_image_extract * extract);
void			free_image_args(t_image_args * args);

void		alloc_and_init_colormap_space_from_src(float **new_colormap, float **source);

inline short checkRequestTimeout(t_image_args *a, Image * img, ImageInfo * image_info);
inline void tidyUp(Image * img, ImageInfo * image_info, FILE *file);
inline void fclose_check(FILE *file);

extern  t_log_plugin	log_plugin;

#endif		/* __IMAGE_EXTRACT__ */
