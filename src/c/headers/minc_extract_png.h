#ifndef __MINC_PNG__
#define __MINC_PNG__

#include <minc2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "thread_pool.h"

typedef	struct	s_options
{
  int		**dim_start_end;
}		t_options;

typedef struct  s_vol
{
  mihandle_t    minc_volume;
  int           dim_nb;
  midimhandle_t *dimensions;
  unsigned int  *size;
  double        **slices;
  char          *path;
  unsigned int	slices_max;
  unsigned int	total_slices_to_do;
  unsigned int	slices_done;
  unsigned int	step;
}               t_vol;

typedef struct	s_arguments
{
  t_vol		*volume;
  t_options	*opt;
  t_thread_pool	*p;
}		t_arguments;



// minc_extract_png.c
void            init_prog(t_vol *volume, char *path_arg);
int		**generate_dims_start_end(t_vol *v, int sx, int ex, int sy,
					  int ey, int sz, int ez);
void            lunch_png_creation(t_vol *vol);

// slice.c
void            *get_all_slices_of_all_dimensions(void *args);
void            get_all_slices_of_one_dimension(t_vol *volume, unsigned long *start,
						int current_dimension, long unsigned int *count, t_arguments *a);
unsigned int            get_slices_max(t_vol *volume);

// png_creator.c
void            print_png(double *hyperslab, t_vol *volume, int current_dimension,
			  unsigned int current_slice, int width, int height);
void            set_grayscale(png_byte *ptr, float val);
void		get_width_height(int *height, int *width, int current_dimension, t_vol *volume);


#define X 0
#define Y 1
#define Z 2

#if defined(DEB)
#define print(format, ...) {						\
  printf("LINE : %i - FILE : %s - FUNCTION : %s | ", __LINE__, __FILE__, __FUNCTION__);       \
  printf(format, __VA_ARGS__);                                                                \
  }
#endif		/*	DEB	*/


#endif		/* __MINC_PNG__ */
