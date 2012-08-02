#ifndef __IMAGE_EXTRACT__
#define __IMAGE_EXTRACT__

#include <minc2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <math.h>

#include <magick/api.h>

#include "core.h"
#include "utils.h"

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
  pthread_cond_t	cond;
  pthread_mutex_t	mut;
}		t_image_extract;

typedef struct  s_image_args
{
  int           **dim_start_end;
  t_vol         *volume;
  t_thread_pool *p;
  t_plugin	*this;
  t_image_extract	*info;
  FILE		*file;
}               t_image_args;

#define X 0
#define Y 1
#define Z 2

unsigned int    get_total_slices_to_do(t_vol *v, int **dim_start_end);
int             **generate_dims_start_end(t_vol *v, int sx, int ex, int sy,
                                          int ey, int sz, int ez);
int             **generate_dims_start_end_thread(t_vol *v, int dim, int start, int end);
t_image_args      *create_args_thread(t_thread_pool *p, t_vol *vol, t_image_extract *image_general, FILE *sock);
void            image_creation_lunch(t_vol *vol, int step, t_thread_pool *p, t_image_extract *image_general, t_plugin *this, FILE *sock);
void			lunch_percent_display(t_thread_pool *p, t_vol *vol, t_image_extract *image_general);
void            *init(void *args);
void            *start(void *args);

unsigned int            get_slices_max(t_vol *volume);
void            *get_all_slices_of_all_dimensions(void *args);
void            get_all_slices_of_one_dimension(t_vol *volume, unsigned long *start, int current_dimension,
                                                long unsigned int *count, t_image_args *a);

void            set_grayscale(unsigned char *ptr, float val);
void            get_width_height(int *height, int *width, int current_dimension, t_vol *volume);
void            print_image(char *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_image_args *a);

void			free_image_extract(t_image_extract * extract);
void			free_image_args(t_image_args * args);

#endif		/* __IMAGE_EXTRACT__ */
