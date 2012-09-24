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
}		t_image_extract;

typedef struct  s_image_args
{
  int			**dim_start_end;
  t_vol			*volume;
  t_thread_pool		*p;
  t_plugin		*this;
  t_image_extract	*info;
  t_tile_requests	*requests;
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

void            set_grayscale(unsigned char *ptr, float val);
void            get_width_height(int *height, int *width, int current_dimension, t_vol *volume);
void            print_image(char *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_image_args *a);

void			free_image_extract(t_image_extract * extract);
void			free_image_args(t_image_args * args);

void		alloc_and_init_colormap_space_from_src(float **new_colormap, float **source);

/*float		colormap[3][25][4] = {{{0, 0, 0, 0},
				    {0.05, 0.46667, 0, 0.05333},
				    {0.1, 0.5333, 0, 0.6},
				    {0.15, 0, 0, 0.6667},
				    {0.2, 0, 0, 0.8667},
				    {0.25, 0, 0.4667, 0.8667},
				    {0.3, 0, 0.6, 0.8667},
				    {0.35, 0, 0.6667, 0.6667},
				    {0.4, 0, 0.6667, 0.5333},
				    {0.45, 0, 0.6, 0},
				    {0.5, 0, 0.7333, 0},
				    {0.55, 0, 0.8667, 0},
				    {0.6, 0, 1, 0},
				    {0.65, 0.7333, 1, 0},
				    {0.7, 0.9333, 0.9333, 0},
				    {0.75, 1, 0.8, 0},
				    {0.8, 1, 0.6, 0},
				    {0.85, 1, 0, 0},
				    {0.9, 0.8667, 0, 0},
				    {0.95, 0.8, 0, 0},
				    {1, 0.8, 0.8, 0.8},
				    {99, 0, 0, 0}},
				   {{0, 0, 0, 0},
				    {0.25, 0.5, 0, 0},
				    {0.5, 1, 0.5, 0},
				    {0.75, 1, 1, 0.5},
				    {1, 1, 1, 1},
				    {99, 0, 0, 0}}};
*/

extern  t_log_plugin	log_plugin;

#endif		/* __IMAGE_EXTRACT__ */
