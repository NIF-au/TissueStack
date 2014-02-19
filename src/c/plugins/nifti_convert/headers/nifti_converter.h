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

unsigned char		*iter_all_pix_and_convert(void *data, unsigned int size, nifti_image *nim)
{
  unsigned int		i = 0;
  unsigned char 	*out;
  unsigned short 	error = 0;

  out = malloc(sizeof(*out) * size * 3);
  for (i=0;i<size;i++) {
	  // keep track of error
	  error = 0;
	  // move start back "data type" number of bytes...
	  data = ((char*)data) + nim->nbyper;
	  // now extract value
	  switch(nim->datatype) {
	  	case NIFTI_TYPE_UINT8: // unsigned char
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = ((unsigned char *) data)[0];
	  		break;
	  	case NIFTI_TYPE_INT8: // signed char
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((char *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_UINT16: // unsigned short
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((unsigned short *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_UINT32: // unsigned int
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((unsigned int *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_INT16: // signed int
	  	case NIFTI_TYPE_INT32: // signed int
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((int *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_UINT64: // unsigned long long
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (double) (((unsigned long long int *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_INT64: // signed long long
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((long long int *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_FLOAT32: //	float
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((float *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_FLOAT64: //	double
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((double *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_FLOAT128: // long double
	  		out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((long double *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_RGB24:
	  		// presumably we have to do nothing ...
	  		out[i] = (unsigned char) (((unsigned char *) data)[0]);
	  		break;
	  	case NIFTI_TYPE_RGBA32:
	  		// presumably we have to skip alpha channel ...
	  		if (i !=0 && (i % 4) == 0) data = ((char*)data) + 1;
	  		break;
	  	case 0:				// UNKNOWN
	  		error = 1;
#ifdef __MINC_NIFTI_CL_CONVERTE__
	  		fprintf(stderr, "Nifti Conversion Error: unknown data type!");
#else
	  		ERROR("Nifti Conversion Error: unknown data type!");
#endif
	  		break;
	  	case DT_BINARY:				// NOT SUPPORTED
	  	case NIFTI_TYPE_COMPLEX64:
	  	case NIFTI_TYPE_COMPLEX128:
	  	case NIFTI_TYPE_COMPLEX256:
	  		error = 1;
#ifdef __MINC_NIFTI_CL_CONVERTE__
	  		fprintf(stderr, "Nifti Conversion Error: unsupported data type!");
#else
	  		ERROR("Nifti Conversion Error: unsupported data type!");
#endif
	  		break;
	  	default:	//	even more unknown
	  		error = 1;
#ifdef __MINC_NIFTI_CL_CONVERTE__
	  		fprintf(stderr, "Nifti Conversion Error: unsupported data type!");
#else
	  		ERROR("Nifti Conversion Error: data type not listed!");
#endif
	  		break;
	  }

	  // check for error
	  if (error) {
		  // free and good bye
		  free(out);
		  return NULL;
	  }
  }

  return out;
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
      h->dim_offset[i] = (unsigned long long)(h->dim_offset[i - 1] + (unsigned long long)((unsigned long long)h->slice_size[i - 1] * (unsigned long long)h->sizes[i - 1]) * 3);
      i++;
    }
  return (h);
}

void conversion_failed_actions(t_args_plug * a, char *id, void * data_out, char * dim_name_char);

extern  t_log_plugin log_plugin;

#endif /* __NIFTI_CONVERTER__ */
