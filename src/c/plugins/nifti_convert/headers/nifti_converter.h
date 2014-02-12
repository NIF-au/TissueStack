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
#ifndef __NIFTI_CONVERTER__
#define __NIFTI_CONVERTER__

#include "core.h"

#include "utils.h"

#include <fcntl.h>
#include <limits.h>
#include <float.h>

#define	NC_NAT 	        0	/* NAT = 'Not A Type' (c.f. NaN) */
#define	NC_BYTE         1	/* signed 1 byte integer */
#define	NC_CHAR 	2	/* ISO/ASCII character */
#define	NC_SHORT 	3	/* signed 2 byte integer */
#define	NC_INT 	        4	/* signed 4 byte integer */
#define NC_LONG         NC_INT  /* deprecated, but required for backward compatibility. */
#define	NC_FLOAT 	5	/* single precision floating point number */
#define	NC_DOUBLE 	6	/* double precision floating point number */
#define	NC_UBYTE 	7	/* unsigned 1 byte int */
#define	NC_USHORT 	8	/* unsigned 2-byte int */
#define	NC_UINT 	9	/* unsigned 4-byte int */
#define	NC_INT64 	10	/* signed 8-byte int */
#define	NC_UINT64 	11	/* unsigned 8-byte int */
#define	NC_STRING 	12	/* string */

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

int		get_sign_nifti(nifti_image *nim) {
  if (nim->datatype == 2 || nim->datatype == 512 || nim->datatype == 768)
    return (MI_PRIV_UNSIGNED);
  else
    return (MI_PRIV_SIGNED);
}

int		get_datatype_nifti(nifti_image *nim) {
  if (nim->datatype == 2 || nim->datatype == 256)
    return(NC_CHAR);
  else if (nim->datatype == 4 || nim->datatype == 512)
    return (NC_SHORT);
  else if (nim->datatype == 8 || nim->datatype == 768)
    return (NC_INT);
  else if (nim->datatype == 16)
    return (NC_FLOAT);
  else
    return (NC_DOUBLE);
}

void		*iter_all_pix_and_convert(void *data_in, unsigned int size, nifti_image *nim)
{
  int		i;
  unsigned char	*data_out;
  double	dvalue = 0.0;
  void		*inptr;
  void		*outptr;
  int		sign;
  int		datatype;
  void		*data;

  datatype = get_datatype_nifti(nim);
  sign = get_sign_nifti(nim);



  if (nim->datatype == 2 || nim->datatype == 256) {
    if (nim->datatype == 2)
      data = (unsigned char*)data_in;
    else
      data = (char*)data_in;
  }
  else if (nim->datatype == 4 || nim->datatype == 512) {
    if (nim->datatype == 512)
      data = (unsigned short*)data_in;
    else
      data = (short*)data_in;
  }
  else if (nim->datatype == 8 || nim->datatype == 768) {
    if (nim->datatype == 768)
      data = (unsigned int*)data_in;
    else
      data = (int*)data_in;
  }
  else if (nim->datatype == 16)
    data = (float *)data_in;
  else
    data = (double*)data_in;


  data_out = malloc((size + 1) * sizeof(*data_out));
  i = 0;
  while (i < size)
    {
      if (nim->datatype == 2 || nim->datatype == 256) {
	if (nim->datatype == 2)
	  inptr = (unsigned char *)(&((unsigned char *)data)[i]);
	else
	  inptr = (char *)(&((char *)data)[i]);
      }
      else if (nim->datatype == 4 || nim->datatype == 512) {
	if (nim->datatype == 512)
	  inptr = (unsigned short *)(&((unsigned short *)data)[i]);
	else
	  inptr = (short *)(&((short *)data)[i]);
      }
      else if (nim->datatype == 8 || nim->datatype == 768) {
	if (nim->datatype == 768)
	  inptr = (unsigned int *)(&((unsigned int *)data)[i]);
	else
	  inptr = (int *)(&((int *)data)[i]);
      }
      else if (nim->datatype == 16)
	inptr = (float *)(&((float *)data)[i]);
      else
	inptr = (double *)(&((double *)data)[i]);
      //      inptr = &data[i];
      outptr = &data_out[i];
      MI_TO_DOUBLE(dvalue, datatype, sign, inptr);
      MI_FROM_DOUBLE(dvalue, NC_CHAR, MI_PRIV_UNSIGNED, outptr);
      i++;
    }
  return (data_out);
}

t_header	*create_header_from_nifti_struct(nifti_image *nifti_volume)
{
  t_header	*h;
  int		i;
  int		j;

  h = malloc(sizeof(*h));
  h->dim_nb = nifti_volume->ndim;

  h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
  h->start = malloc(h->dim_nb * sizeof(*h->start));
  h->steps = malloc(h->dim_nb * sizeof(*h->steps));
  h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
  h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
  h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));

  h->slice_max = (nifti_volume->dim[1] * nifti_volume->dim[2] > nifti_volume->dim[2] * nifti_volume->dim[3] ?
		  (nifti_volume->dim[1] * nifti_volume->dim[2] > nifti_volume->dim[1] * nifti_volume->dim[3] ? nifti_volume->dim[1] * nifti_volume->dim[2] : nifti_volume->dim[1] * nifti_volume->dim[3]) :
		  (nifti_volume->dim[2] * nifti_volume->dim[3] > nifti_volume->dim[1] * nifti_volume->dim[3] ? nifti_volume->dim[2] * nifti_volume->dim[3] : nifti_volume->dim[1] * nifti_volume->dim[3]));

  i = 0;
  while (i < h->dim_nb)
    {
      h->sizes[i] = nifti_volume->dim[i + 1];
      h->start[i] = nifti_volume->sto_xyz.m[i][3];
      h->steps[i] = nifti_volume->pixdim[i + 1];
      h->dim_name[i] = strdup("xspace");
      h->dim_name[i][0] = 'x' + i;

      h->slice_size[i] = 1;
      j = 1;
      while (j < h->dim_nb + 1)
	{
	  if ((j - 1) != i)
	    h->slice_size[i] *= nifti_volume->dim[j];
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

#endif /* __NIFTI_CONVERTER__ */
