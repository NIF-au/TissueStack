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
#ifndef __MINC_TOOL_CORE__
#define __MINC_TOOL_CORE__

#include "core.h"
#include "utils.h"

#include <sys/stat.h>

void failureToConvertSlice(t_tissue_stack * t, unsigned char * data, char * dim_name_char, char * id) {
#ifdef __MINC_NIFTI_CL_CONVERTE__
				fprintf(stderr, "Could not convert slice");
#else
				ERROR("Could not convert slice");
				t->percent_cancel(id, t);
#endif
	if (data != NULL) free(data);
	if (dim_name_char != NULL) free(dim_name_char);
}

unsigned int            get_slices_max(t_vol *volume)
{
  // get the larger number of slices possible
  if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[X]))
    {
      if ((volume->size[X] * volume->size[Y]) > (volume->size[Z] * volume->size[Y]))
        return (volume->size[X] * volume->size[Y]);
    }
  else if ((volume->size[Z] * volume->size[X]) > (volume->size[Z] * volume->size[Y]))
    return (volume->size[Z] * volume->size[X]);
  return (volume->size[Z] * volume->size[Y]);
}

unsigned char * extractDataFromMincVolume(t_vol * volume, const unsigned long start[], const unsigned long count[], int size) {

	mitype_t		minc_type;
	misize_t		minc_type_size;
	void 			*in = NULL;
	unsigned char 	*out = NULL;
	unsigned int		i = 0;
	unsigned short 	error = 0;

	if (miget_data_type(volume->minc_volume, &minc_type) != MI_NOERROR) return NULL;
	if (miget_data_type_size(volume->minc_volume, &minc_type_size) != MI_NOERROR) return NULL;

	// create generic data buffer of proper size
	// TODO: allocate once outside, then deallocate at the very end
	in = malloc(size*minc_type_size);
	memset(in, '\0', size*minc_type_size);

	// read hyperslab
	if (miget_real_value_hyperslab(volume->minc_volume, minc_type, start, count, in) != MI_NOERROR) {
		if (in != NULL) free(in);
		return NULL;
	}

	out = malloc(sizeof(*out) * size * 3);
	for (i=0;i<size;i++) {
		// keep track of error
		error = 0;
		// move start back "data type" number of bytes...
		in = ((char*)in) + minc_type_size;

		// now extract value
		switch(minc_type) {
			case MI_TYPE_UBYTE: // unsigned char
				out[i*3+0] = out[i*3+1] = out[i*3+2] = ((unsigned char *) in)[0];
				break;
			case MI_TYPE_BYTE: // signed char
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((char *) in)[0]);
				break;
			case MI_TYPE_USHORT: // unsigned short
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((unsigned short *) in)[0]);
				break;
			case MI_TYPE_SHORT: // signed int
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((short *) in)[0]);
				break;
			case MI_TYPE_UINT: // unsigned int
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((unsigned int *) in)[0]);
				break;
			case MI_TYPE_INT: // signed int
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((int *) in)[0]);
				break;
			case MI_TYPE_FLOAT: //	float
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((float *) in)[0]);
				break;
			case MI_TYPE_DOUBLE: //	double
				out[i*3+0] = out[i*3+1] = out[i*3+2] = (unsigned char) (((double *) in)[0]);
				break;
			case MI_TYPE_UNKNOWN:				// UNKNOWN
				error = 1;
#ifdef __MINC_NIFTI_CL_CONVERTE__
				fprintf(stderr, "Minc Conversion Error: unknown data type!");
#else
				ERROR("Minc Conversion Error: unknown data type!");
#endif
				break;
			case MI_TYPE_STRING:				// NOT SUPPORTED
			case MI_TYPE_SCOMPLEX:
			case MI_TYPE_ICOMPLEX:
			case MI_TYPE_FCOMPLEX:
			case MI_TYPE_DCOMPLEX:
				error = 1;
#ifdef __MINC_NIFTI_CL_CONVERTE__
				fprintf(stderr, "Minc Conversion Error: unsupported data type!");
#else
				ERROR("Minc Conversion Error: unsupported data type!");
#endif
				break;
			default:	//	even more unknown
				error = 1;
#ifdef __MINC_NIFTI_CL_CONVERTE__
				fprintf(stderr, "Minc Conversion Error: data type not listed!");
#else
				ERROR("Minc Conversion Error: data type not listed!");
#endif
				break;
		  }

		  // check for error
		  if (error) {
			  // free and good bye
			  if (in != NULL) free(in);
			  if (out != NULL) free(out);
			  return NULL;
		  }
	}
	// release in data
	//if (in != NULL) free(in);

	return out;
}

void dim_loop(int fd, int dimensions_nb, t_vol *volume, t_tissue_stack *t, char *id_percent, int slice_resume, int dimension_resume) {
	int dim = 0;
	int slice = 0;
	int this_slice = 0;
	int size;
	unsigned char *data = NULL;
	int i = 0, j = 0;
	unsigned long *start;
	long unsigned int *count;
	short cancel = 0;
	char *dim_name_char = NULL;
	Image *img = NULL;
	PixelPacket *pixels;
	int width = 0;
	int height = 0;

	if (dimension_resume > -1)
		dim = dimension_resume;
	start = malloc(volume->dim_nb * sizeof(*start));
	count = malloc(volume->dim_nb * sizeof(*count));
	start[0] = start[1] = start[2] = 0;
	i = 0;
	while (i < volume->dim_nb) {
		count[i] = volume->size[i];
		i++;
	}

	dim_name_char = malloc(volume->dim_nb * sizeof(*dim_name_char));
	for (j = 0; j < volume->dim_nb; j++)
		dim_name_char[j] = volume->dim_name[j][0];

	while (dim < dimensions_nb && cancel == 0) { // DIMENSION LOOP
		size = (dim == 0 ?
				(volume->size[2] * volume->size[1]) :
				(dim == 1 ?
						(volume->size[0] * volume->size[2]) :
						(volume->size[0] * volume->size[1])));
		slice = volume->size[dim];
		if (slice_resume != -1) {
			this_slice = slice_resume;
			slice_resume = -1;
		} else
			this_slice = 0;
		count[dim] = 1;
		while (this_slice < slice && cancel == 0) { // SLICE LOOP
			img = NULL;
			data = NULL;
			start[dim] = this_slice;

			data = extractDataFromMincVolume(volume, start, count, size);
			if (data == NULL) {
				failureToConvertSlice(t, NULL, dim_name_char, id_percent);
				return;
			}

			// convert to image and perform proper orientation corrections
			get_width_height(&height, &width, dim, volume->dim_nb, dim_name_char, volume->size);
			volume->raw_format = MINC;
			volume->raw_data_type = RGB_24BIT;
			img = extractSliceDataAtProperOrientation(
					volume->raw_format, volume->raw_data_type, dim_name_char, dim, data, width, height, NULL);
			if (img == NULL) {
				failureToConvertSlice(t, data, dim_name_char, id_percent);
				return;
			}

			// extract pixel info, looping over values
			pixels = GetImagePixels(img, 0, 0, width, height);
			if (pixels == NULL) {
				if (img != NULL) DestroyImage(img);
				failureToConvertSlice(t, data, dim_name_char, id_percent);
				return;
			}
			// sync with data
			for (j = 0; j < size; j++) {
				// graphicsmagic quantum depth correction
				if (QuantumDepth != 8 && img->depth == QuantumDepth) {
					data[j * 3 + 0] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].red);
					data[j * 3 + 1] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].green);
					data[j * 3 + 2] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].blue);
					continue;
				} // no correction needed
				data[j * 3 + 0] = (unsigned char) pixels[(width * i) + j].red;
				data[j * 3 + 1] = (unsigned char) pixels[(width * i) + j].green;
				data[j * 3 + 2] = (unsigned char) pixels[(width * i) + j].blue;
			}
			write(fd, data, size * 3);

#ifdef __MINC_NIFTI_CL_CONVERTE__
			printf("Slice = %i / %i - dim = %i\r", this_slice, (int) volume->size[dim], dim);
			fflush(stdout);
#else
			DEBUG("Slice = %i - dim = %i", this_slice, dim);
			t->percent_add(1, id_percent, t);
			cancel = t->is_percent_paused_cancel(id_percent, t);
#endif
			this_slice++;
			if (img != NULL) DestroyImage(img);
		}

		start[dim] = 0;
		count[dim] = volume->size[dim];
		dim++;
		if (data != NULL) free(data);
	}
	if (dim_name_char != NULL) free(dim_name_char);
}

int		get_nb_total_slices_to_do(t_vol *volume)
{
  int		i = 0;
  int		count = 0;

  while (i < volume->dim_nb)
    {
      count += volume->size[i];
      i++;
    }
  return (count);
}

t_vol *init_get_volume_from_minc_file(char *path) {
	t_vol *volume;
	int result;

	volume = malloc(sizeof(*volume));
	volume->path = path;
	volume->dim_nb = 3;
	if (volume->path == NULL)
		return (NULL);
	// open the minc file
	if ((result = miopen_volume(volume->path, MI2_OPEN_READ,
			&volume->minc_volume)) != MI_NOERROR) {
		fprintf(stderr, "Error opening input file: %d.\n", result);
		return (NULL);
	}

	if ((result = miget_volume_dimension_count(volume->minc_volume, 0, 0,
			&volume->dim_nb)) != MI_NOERROR) {
		fprintf(stderr, "Error getting number of dimensions: %d.\n", result);
		return (NULL);
	}

	volume->dimensions = malloc(volume->dim_nb * sizeof(*volume->dimensions));
	volume->starts = malloc(volume->dim_nb * sizeof(*volume->starts));
	volume->steps = malloc(volume->dim_nb * sizeof(*volume->steps));
	volume->size = malloc(volume->dim_nb * sizeof(*volume->size));
	volume->dim_name = malloc(volume->dim_nb * sizeof(*volume->dim_name));

	// get the volume dimensions
	if ((result = miget_volume_dimensions(volume->minc_volume,
			MI_DIMCLASS_SPATIAL, MI_DIMATTR_ALL, MI_DIMORDER_FILE,
			volume->dim_nb, volume->dimensions)) == MI_ERROR) {
		fprintf(stderr, "Error getting dimensions: %d.\n", result);
		return (NULL);
	}
	// get the size of each dimensions

	if ((result = miget_dimension_size(volume->dimensions[0], &volume->size[0]))
			!= MI_NOERROR) {
		fprintf(stderr, "Error getting dimensions size: %d.\n", result);
		return (NULL);
	}

	if ((result = miget_dimension_size(volume->dimensions[1], &volume->size[1]))
			!= MI_NOERROR) {
		fprintf(stderr, "Error getting dimensions size: %d.\n", result);
		return (NULL);
	}

	if ((result = miget_dimension_size(volume->dimensions[2], &volume->size[2]))
			!= MI_NOERROR) {
		fprintf(stderr, "Error getting dimensions size: %d.\n", result);
		return (NULL);
	}

	if ((result = miget_dimension_starts(volume->dimensions, 0, volume->dim_nb,
			volume->starts)) != MI_NOERROR) {
		fprintf(stderr, "Error getting dimensions start: %d.\n", result);
		return (NULL);
	}
	if ((result = miget_dimension_separations(volume->dimensions, 0,
			volume->dim_nb, volume->steps)) != MI_NOERROR) {
		fprintf(stderr, "Error getting dimensions steps: %d.\n", result);
		return (NULL);
	}
	if (miget_dimension_name(volume->dimensions[0], &volume->dim_name[0])
			!= MI_NOERROR
			|| miget_dimension_name(volume->dimensions[1], &volume->dim_name[1])
					!= MI_NOERROR
			|| miget_dimension_name(volume->dimensions[2],
					&volume->dim_name[2])) {
		fprintf(stderr, "Error getting dimensions name.\n");
		return (NULL);
	}
	// get slices_max
	volume->slices_max = get_slices_max(volume);
	volume->next = NULL;
	return (volume);
}

t_header	*create_header_from_minc_struct(t_vol *minc_volume)
{
  t_header	*h;
  int		i;
  int		j;

  h = malloc(sizeof(*h));

  h->dim_nb = minc_volume->dim_nb;

  h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
  h->start = malloc(h->dim_nb * sizeof(*h->start));
  h->steps = malloc(h->dim_nb * sizeof(*h->steps));
  h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
  h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
  h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));

  h->slice_max = minc_volume->slices_max;

  i = 0;
  while (i < h->dim_nb)
    {
      h->sizes[i] = minc_volume->size[i];
      h->start[i] = minc_volume->starts[i];
      h->steps[i] = minc_volume->steps[i];
      h->dim_name[i] = strdup(minc_volume->dim_name[i]);

      h->slice_size[i] = 1;
      j = 0;
      while (j < h->dim_nb)
	{
	  if (j != i)
	    h->slice_size[i] *= minc_volume->size[j];
	  j++;
	}
      i++;
    }

  h->dim_offset[0] = 0;
  i = 1;
  while (i < h->dim_nb)
    {
      h->dim_offset[i] =
    		  (unsigned long long)(h->dim_offset[i - 1] +
    				  (unsigned long long)((unsigned long long)h->slice_size[i - 1] * (unsigned long long)h->sizes[i - 1]) * 3);
      i++;
    }
  return (h);
}

extern  t_log_plugin log_plugin;

#endif /* __MINC_TOOL_CORE__ */
