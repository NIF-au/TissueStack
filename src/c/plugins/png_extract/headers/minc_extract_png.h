#ifndef __MINC_PNG__
#define __MINC_PNG__

#include <minc2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include <magick/api.h>

#include "core.h"

typedef	struct	s_png_extract
{
  int		**dim_start_end;
  unsigned int	total_slices_to_do;
  unsigned int	slices_done;
  unsigned int	step;
  float		scale;
  int		quality;
  int		h_position;
  int		w_position;
  int		h_position_end;
  int		w_position_end;
  char		*service;
  int		square_size;
}		t_png_extract;

typedef struct  s_png_args
{
  int           **dim_start_end;
  t_vol         *volume;
  t_thread_pool *p;
  t_plugin	*this;
  t_png_extract	*info;
  FILE		*file;
}               t_png_args;

#define X 0
#define Y 1
#define Z 2

unsigned int    get_total_slices_to_do(t_vol *v, int **dim_start_end);
int             **generate_dims_start_end(t_vol *v, int sx, int ex, int sy,
                                          int ey, int sz, int ez);
int             **generate_dims_start_end_thread(t_vol *v, int dim, int start, int end);
t_png_args      *create_args_thread(t_thread_pool *p, t_vol *vol, t_png_extract *png_general, t_plugin *this, FILE *sock);
void            png_creation_lunch(t_vol *vol, int step, t_thread_pool *p, t_png_extract *png_general, t_plugin *this, FILE *sock);
void            *init(void *args);
void            *start(void *args);

unsigned int            get_slices_max(t_vol *volume);
void            *get_all_slices_of_all_dimensions(void *args);
void            get_all_slices_of_one_dimension(t_vol *volume, unsigned long *start, int current_dimension,
                                                long unsigned int *count, t_png_args *a);

void            set_grayscale(png_byte *ptr, float val);
void            get_width_height(int *height, int *width, int current_dimension, t_vol *volume);
void            print_png(char *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_png_args *a);

void		image_calculus_scaled_entire(int width, int height, double *hyperslab,
					     float scale, int quality, png_structp *png_ptr);

void		image_calculus_scaled_chunk_lowres_images(int width, int height, double *hyperslab,
							  float scale, int quality, png_structp *png_ptr,
							  int square_size, int h_position, int w_position);
void		image_calculus_scaled_chunk_lowres_tiles(int width, int height, double *hyperslab,
							 float scale, int quality, png_structp *png_ptr,
							 int square_size, int h_position, int w_position);
int		image_calculus_scaled_chunk(int width, int height, double *hyperslab,
						   float scale, int quality, png_structp *png_ptr,
						   int h_position, int w_position,
						   int h_position_end, int w_position_end);
void		image_calculus_scaled_chunk_tiles(int width, int height, double *hyperslab,
						  float scale, int quality, png_structp *png_ptr,
						  int square_size, int h_position, int w_position);
/*
double		*image_calculus_scaled_chunk_data(int width, int height, double *hyperslab,
						  float scale, int quality, png_structp *png_ptr,
						  int square_size, int h_position, int w_position);
*/
double		*image_calculus_scaled_chunk_data(int width, int height, double *hyperslab,
						  float scale, int quality, png_structp *png_ptr,
						  int h_position, int w_position, int h_position_end, int w_position_end);


#endif		/* __MINC_PNG__ */
