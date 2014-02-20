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
  unsigned char 	*out = NULL;
  unsigned short 	error = 0;

  out = malloc(sizeof(*out) * size * 3);
  for (i=0;i<size;i++) {
	  // keep track of error
	  error = 0;
	  // move start back "data type" number of bytes...
	  if (i != 0) data = ((char*)data) + nim->nbyper;
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

void conversion_failed_actions(t_args_plug * a, char *id, void * data_out, char * dim_name_char) {
	if (data_out != NULL)	free(data_out);
	if (dim_name_char != NULL) free(dim_name_char);

#ifdef __MINC_NIFTI_CL_CONVERTE__
	fprintf(stderr,"Could not convert slice");
#else
	//a->general_info->percent_cancel(id, a->general_info);
	ERROR("Could not convert slice");
#endif
}

void	convertNifti0(t_args_plug *a, nifti_image	*nim, t_header *h, int fd, int i, char *id_percent) {
	int				j=0;
	int				slice = 0;
	int				ret;
	int				sizes[3];
	int				dims[8] = { 0,  -1, -1, -1, -1, -1, -1, -1 };
	void 			*data_in = NULL;
	unsigned char	*data_out = NULL;
	Image			*img = NULL;
	PixelPacket 	*pixels;
	char 			*dim_name_char = NULL;
	int 			width = 0;
	int 			height = 0;
	int				nslices;
	unsigned int	size_per_slice;
	int				cancel = 0;

	sizes[0] = nim->dim[1];
	sizes[1] = nim->dim[2];
	sizes[2] = nim->dim[3];

	dim_name_char = malloc(h->dim_nb * sizeof(*dim_name_char));
	for (j=0;j<h->dim_nb;j++)
		dim_name_char[j] = h->dim_name[j][0];

	while (i <= nim->dim[0] && cancel == 0) {	// DIMENSION LOOP
		slice = 0;
		nslices = sizes[i - 1];
		size_per_slice = h->slice_size[i - 1];
		while (slice < nslices && cancel == 0) { // SLICE LOOP
			img = NULL;
			data_in = NULL;
			data_out = NULL;
			dims[i] = slice;
			if ((ret = nifti_read_collapsed_image(nim, dims, (void*) &data_in))	< 0) {
				if (dim_name_char != NULL)	free(dim_name_char);

				#ifdef __MINC_NIFTI_CL_CONVERTE__
				fprintf(stderr,"Error reading Nifti data!");
#else
				ERROR("Error reading Nifti data!");
#endif

				return;
			}

			data_out = iter_all_pix_and_convert(data_in, size_per_slice, nim);
			free(data_in);
			if (data_out == NULL) {
				conversion_failed_actions(NULL, NULL,data_out, dim_name_char);
				return;
			}

			get_width_height(&height, &width, i - 1, h->dim_nb,	dim_name_char, h->sizes);
			img = extractSliceDataAtProperOrientation(NIFTI, RGB_24BIT, dim_name_char,	i - 1, data_out, width, height, NULL);
			if (img == NULL) {
				conversion_failed_actions(NULL, NULL,data_out, dim_name_char);
				return;
			}

			// extract pixel info, looping over values
			pixels = GetImagePixels(img, 0, 0, width, height);
			if (pixels == NULL) {
				if (img != NULL) DestroyImage(img);
				conversion_failed_actions(NULL, NULL,data_out, dim_name_char);
				return;
			}

			// sync with data
			for (j = 0; j < size_per_slice; j++) {
				// graphicsmagic quantum depth correction
				if (QuantumDepth != 8 && img->depth == QuantumDepth) {
					data_out[j*3+0] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].red);
					data_out[j*3+1] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].green);
					data_out[j*3+2] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].blue);
					continue;
				} // no correction needed
				data_out[j*3+0] = (unsigned char) pixels[(width * i) + j].red;
				data_out[j*3+1] = (unsigned char) pixels[(width * i) + j].green;
				data_out[j*3+2] = (unsigned char) pixels[(width * i) + j].blue;
			}
			// write into file
			write(fd, data_out, size_per_slice * 3);
			// increment slice
			slice++;

			// tidy up
			if (img != NULL)
				DestroyImage(img);
			free(data_out);

#ifdef __MINC_NIFTI_CL_CONVERTE__
			printf("Slice %i / %i of plane '%c'       \r",	slice, nslices, dim_name_char[(i - 1)]);
			fflush(stdout);
#else
			a->general_info->percent_add(1, id_percent, a->general_info);
			cancel = a->general_info->is_percent_paused_cancel(id_percent,	a->general_info);
			DEBUG("Slice n %i on dimension %i slicenb = %i -- cancel = %i",	slice, (i - 1), nslices, cancel);
#endif
		}
		dims[i] = -1;
		i++;
	}

	// tidy up
	if (dim_name_char != NULL) free(dim_name_char);

#ifndef __MINC_NIFTI_CL_CONVERTE__
	if (cancel == 0) INFO("Conversion: NIFTI: %s to RAW: %s ==> DONE", a->commands[0], a->commands[1]);
#endif
}


extern  t_log_plugin log_plugin;

#endif /* __NIFTI_CONVERTER__ */
