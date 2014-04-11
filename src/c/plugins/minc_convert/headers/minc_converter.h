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
	if (data != NULL)
		free(data);
	if (dim_name_char != NULL)
		free(dim_name_char);
}

void extractDataFromMincVolume(
		t_vol * volume, const unsigned long start[], const unsigned long count[], mitype_t minc_type,
		misize_t minc_type_size, void * in, unsigned char * out, int size, short rgb_channel) {
	unsigned int i = 0;
	unsigned char val = 0;

	// read hyperslab as unsigned byte and let minc do the dirty deeds of conversion
	if (miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE, start,
			count, in) != MI_NOERROR) {
		if (out != NULL) {
			free(out);
			out = NULL;
		}
		return;
	}
	for (i = 0; i < size; i++) {
		val = ((unsigned char *) in)[i];
		if (rgb_channel < 0) out[i * 3 + 0] = out[i * 3 + 1] = out[i * 3 + 2] = val;
		else out[i * 3 + rgb_channel] = val;
	}
}

void dim_loop(int fd, int dimensions_nb, t_vol *volume, t_tissue_stack *t,
		char *id_percent, int slice_resume, int dimension_resume, t_args_plug * a, t_header * h) {
	mitype_t minc_type;
	misize_t minc_type_size;
	void *buffer = NULL;
	int dim = (dimension_resume < 0) ? 0 : dimension_resume;
	int slice = 0;
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
	short rgb_channel = 0;
	short rgb_total = h->channels > 0 ? h->channels : 1;

	// define start and length for hyperslap read
	int length = h->channels > 0 ? h->dim_nb+1 : h->dim_nb;
	start = malloc(length * sizeof(*start));
	count = malloc(length * sizeof(*count));
	while (i<length) {
		start[i] = 0;
		if (i < 3) {
			start[i] = 0;
			count[i] = h->sizes[i];
		} else count[i] = 1;
		i++;
	}
	i = 0;

	// get minc volume params
	if (miget_data_type(volume->minc_volume, &minc_type) != MI_NOERROR) {
		failureToConvertSlice(t, NULL, NULL, id_percent);
		return;
	}

	if (miget_data_type_size(volume->minc_volume, &minc_type_size) != MI_NOERROR) {
		failureToConvertSlice(t, NULL, NULL, id_percent);
		return;
	}

	// plane lookup
	dim_name_char = malloc(h->dim_nb * sizeof(*dim_name_char));
	for (j = 0; j < h->dim_nb; j++)
		dim_name_char[j] = h->dim_name[j][0];

	while (dim < dimensions_nb && cancel == 0) { // DIMENSION LOOP
		size = (dim == 0 ?
				(h->sizes[2] * h->sizes[1]) :
				(dim == 1 ?
						(h->sizes[0] * h->sizes[2]) :
						(h->sizes[0] * h->sizes[1])));
		count[dim] = 1;

		// create data buffers of appropriate size
		buffer = malloc(size * minc_type_size);
		data = malloc(sizeof(*data) * size * 3);

		// reset slice or resume
		if (slice_resume > 0) {
			slice = slice_resume;
			slice_resume = -1;
		} else {
			slice = 0;
		}

		while (slice < h->sizes[dim] && cancel == 0) { // SLICE LOOP
			img = NULL;
			rgb_channel = 0;
			start[dim] = slice;

			// read data (including rgb)
			while (rgb_channel < rgb_total) {
				if (h->channels > 0) start[h->dim_nb] = rgb_channel;
				extractDataFromMincVolume(
						volume, start, count, minc_type,
						minc_type_size, buffer, data, size,
						h->channels > 0 ? rgb_channel : -1);

				if (data == NULL) {
					failureToConvertSlice(t, NULL, dim_name_char, id_percent);
					return;
				}

				rgb_channel++;
			}

			// convert to image and perform proper orientation corrections
			get_width_height(&height, &width, dim, h->dim_nb, dim_name_char, h->sizes);
			volume->raw_format = MINC;
			volume->raw_data_type = RGB_24BIT;
			img = extractSliceDataAtProperOrientation(volume->raw_format,
					volume->raw_data_type, dim_name_char, dim, data, width,
					height, NULL);
			if (img == NULL) {
				failureToConvertSlice(t, data, dim_name_char, id_percent);
				return;
			}

			// extract pixel info, looping over values
			pixels = GetImagePixels(img, 0, 0, width, height);
			if (pixels == NULL) {
				if (img != NULL)
					DestroyImage(img);
				failureToConvertSlice(t, data, dim_name_char, id_percent);
				return;
			}
			// sync with data
			for (j = 0; j < size; j++) {
				// graphicsmagic quantum depth correction
				if (QuantumDepth != 8 && img->depth == QuantumDepth) {
					data[j * 3 + 0] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[j].red);
					data[j * 3 + 1] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[j].green);
					data[j * 3 + 2] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[j].blue);
					continue;
				} // no correction needed
				data[j * 3 + 0] = (unsigned char) pixels[j].red;
				data[j * 3 + 1] = (unsigned char) pixels[j].green;
				data[j * 3 + 2] = (unsigned char) pixels[j].blue;
			}
			write(fd, data, size * 3);

#ifdef __MINC_NIFTI_CL_CONVERTE__
			printf("Slice %i / %i of plane '%c'       \r", slice, (int) h->sizes[dim], dim_name_char[dim]);
			fflush(stdout);
#else
			DEBUG("Slice %i / %i [%c]", slice, (int ) h->sizes[dim],
					dim_name_char[dim]);
			t->percent_add(1, id_percent, t);
			cancel = t->is_percent_paused_cancel(id_percent, t);
#endif
			slice++;
			if (img != NULL)
				DestroyImage(img);
		}

		start[dim] = 0;
		count[dim] = h->sizes[dim];
		dim++;

		// tidy up
		if (buffer != NULL)
			free(buffer);
		if (data != NULL)
			free(data);
	}
	if (dim_name_char != NULL)
		free(dim_name_char);


#ifdef __MINC_NIFTI_CL_CONVERTE__
	printf("\nConversion finished!\n");
#else
	if (cancel == 0) {
		INFO("Conversion: MINC: %s to RAW: %s ==> DONE", a->commands[0], a->commands[1]);
	} else {
		INFO("Conversion: MINC: %s to RAW: %s ==> CANCELED", a->commands[0], a->commands[1]);
	}
#endif

}

int get_nb_total_slices_to_do(t_header *header) {
	int i = 0;
	int count = 0;

	while (i < header->dim_nb) {
		count += header->sizes[i];
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

t_header *create_header_from_minc_struct(t_vol *minc_volume) {
	t_header *h;
	int i;
	int j;

	h = malloc(sizeof(*h));

	h->dim_nb = minc_volume->dim_nb;
	if (h->dim_nb > 3) { // this is how we handle potential RGB
		h->channels = 3;
		h->dim_nb = 3; // internally we use this to determine our loop length
	} else h->channels = 0;

	h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
	h->sizes_isotropic = malloc(h->dim_nb * sizeof(*h->sizes_isotropic));
	h->start = malloc(h->dim_nb * sizeof(*h->start));
	h->steps = malloc(h->dim_nb * sizeof(*h->steps));
	h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
	h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
	h->dim_offset_isotropic = malloc(h->dim_nb * sizeof(*h->dim_offset_isotropic));
	h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));
	h->slice_size_isotropic = malloc(h->dim_nb * sizeof(*h->slice_size_isotropic));

	h->slice_max = minc_volume->slices_max;

	i = 0;
	while (i < h->dim_nb) {
		h->sizes[i] = minc_volume->size[i];
		h->sizes_isotropic[i] = h->sizes[i]; // copy over for now and correct it below (if anisotropic)
		h->start[i] = minc_volume->starts[i];
		h->steps[i] = minc_volume->steps[i];
		h->dim_name[i] = strdup(minc_volume->dim_name[i]);

		h->slice_size[i] = 1;
		j = 0;
		while (j < h->dim_nb) {
			if (j != i)
				h->slice_size[i] *= minc_volume->size[j];
			// copy over for now and correct it below (if anisotropic)
			h->slice_size_isotropic[i] = h->slice_size[i];
			j++;
		}
		i++;
	}

	// check isotropy
	determine_isotropy(h);
	if (h->is_isotropic == NO) { // store isotropic dimensions separately for future use
		i = 0;
		while (i < h->dim_nb) {
			h->slice_size_isotropic[i] = 1;
			h->sizes_isotropic[i] = (unsigned int) round(
					(double) h->sizes_isotropic[i] * h->steps[i]);
			i++;
		}
	}

	// calculate offsets
	i = 0;
	while (i < h->dim_nb) {
		if (h->is_isotropic == NO) { // compute slice_sizes for anisotropic
			j = 0;
			while (j < h->dim_nb) {
				if (j != i)	h->slice_size_isotropic[i] *= h->sizes_isotropic[j];
				j++;
			}
		}

		if (i == 0) { // offset for first dimension: 0
			h->dim_offset[0] = 0;
			h->dim_offset_isotropic[0] = 0;
			i++;
			continue;
		}

		// following offsets
		h->dim_offset[i] =
				(unsigned long long) (h->dim_offset[i - 1]
						+ (unsigned long long) ((unsigned long long) h->slice_size[i
								- 1] * (unsigned long long) h->sizes[i - 1]) * 3);

		if (h->is_isotropic == NO) // calculate isotropic offsets
			h->dim_offset_isotropic[i] =
					(unsigned long long) (h->dim_offset_isotropic[i - 1]
							+ (unsigned long long) ((unsigned long long) h->slice_size_isotropic[i
									- 1]
									* (unsigned long long) h->sizes_isotropic[i
											- 1]) * 3);
		else
			// merely copy over
			h->dim_offset_isotropic[i] = h->dim_offset[i];
		i++;
	}

	return (h);
}

extern t_log_plugin log_plugin;

#endif /* __MINC_TOOL_CORE__ */
