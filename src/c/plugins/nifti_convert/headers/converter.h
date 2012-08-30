#ifndef __NIFTI_CONVERTER__
#define __NIFTI_CONVERTER__

#include <nifti1_io.h>
#include <fcntl.h>
#include <limits.h>
#include <netcdf.h>
#include <minc2.h>
#include <float.h>
#include "core.h"

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

void		*iter_all_pix_and_convert(void *data_in, unsigned int size, nifti_image *nim);
t_header	*create_header_from_nifti_struct(nifti_image *nifti_volume);
void		write_header_into_file(int fd, t_header *h);

#define MI_PRIV_UNSIGNED 0
#define MI_PRIV_SIGNED 1

#define ROUND( x ) ((long) ((x) + ( ((x) >= 0) ? 0.5 : (-0.5) ) ))
#define _MAX( x, y )  ( ((x) >= (y)) ? (x) : (y) )
#define _MIN( x, y )  ( ((x) >= (y)) ? (y) : (x) )

#define MI_TO_DOUBLE(dvalue, type, sign, ptr)	{			\
  switch (type) {							\
  case NC_BYTE :							\
  case NC_CHAR:								\
    switch (sign) {							\
    case MI_PRIV_UNSIGNED :						\
      dvalue = (double) *((unsigned char *) ptr); break;		\
    case MI_PRIV_SIGNED :						\
      dvalue = (double) *((signed char *) ptr); break;			\
    }									\
    break;								\
  case NC_SHORT :							\
    switch (sign) {							\
    case MI_PRIV_UNSIGNED :						\
      dvalue = (double) *((unsigned short *) ptr); break;		\
    case MI_PRIV_SIGNED :						\
      dvalue = (double) *((signed short *) ptr); break;			\
    }									\
    break;								\
  case NC_INT :								\
    switch (sign) {							\
    case MI_PRIV_UNSIGNED :						\
      dvalue = (double) *((unsigned int *) ptr); break;			\
    case MI_PRIV_SIGNED :						\
      dvalue = (double) *((signed int  *) ptr); break;			\
    }									\
    break;								\
  case NC_FLOAT :							\
    dvalue = (double) *((float *) ptr);					\
    break;								\
  case NC_DOUBLE :							\
    dvalue = (double) *((double *) ptr);				\
    break;								\
  }									\
  }

#define MI_FROM_DOUBLE(dvalue, type, sign, ptr)	{			\
  switch (type) {							\
  case NC_BYTE :							\
  case NC_CHAR :							\
    switch (sign) {							\
    case MI_PRIV_UNSIGNED :						\
      dvalue = _MAX(0, dvalue);						\
      dvalue = _MIN(UCHAR_MAX, dvalue);					\
      *((unsigned char *) ptr) = ROUND(dvalue);				\
      break;								\
    case MI_PRIV_SIGNED :						\
      dvalue = _MAX(SCHAR_MIN, dvalue);					\
      dvalue = _MIN(SCHAR_MAX, dvalue);					\
      *((signed char *) ptr) = ROUND(dvalue);				\
      break;								\
    }									\
    break;								\
  case NC_SHORT :							\
    switch (sign) {							\
    case MI_PRIV_UNSIGNED :						\
      dvalue = _MAX(0, dvalue);						\
      dvalue = _MIN(USHRT_MAX, dvalue);					\
      *((unsigned short *) ptr) = ROUND(dvalue);			\
      break;								\
    case MI_PRIV_SIGNED :						\
      dvalue = _MAX(SHRT_MIN, dvalue);					\
      dvalue = _MIN(SHRT_MAX, dvalue);					\
      *((signed short *) ptr) = ROUND(dvalue);				\
      break;								\
    }									\
    break;								\
  case NC_INT :								\
    switch (sign) {							\
    case MI_PRIV_UNSIGNED :						\
      dvalue = _MAX(0, dvalue);						\
      dvalue = _MIN(UINT_MAX, dvalue);					\
      *((unsigned int *) ptr) = ROUND(dvalue);				\
      break;								\
    case MI_PRIV_SIGNED :						\
      dvalue = _MAX(INT_MIN, dvalue);					\
      dvalue = _MIN(INT_MAX, dvalue);					\
      *((signed int *) ptr) = ROUND(dvalue);				\
      break;								\
    }									\
    break;								\
  case NC_FLOAT :							\
    dvalue = _MAX(-FLT_MAX,dvalue);					\
    *((float *) ptr) = _MIN(FLT_MAX,dvalue);				\
    break;								\
  case NC_DOUBLE :							\
    *((double *) ptr) = dvalue;						\
    break;								\
  }									\
  }


#endif /* __NIFTI_CONVERTER__ */
