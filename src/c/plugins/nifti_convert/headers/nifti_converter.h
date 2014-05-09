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

void setGlobalMinMax(nifti_image *nim, t_header *h) {
	void *data_in = NULL, *in = NULL;
	int dims[8] = { 0, -1, -1, -1, -1, -1, -1, -1 };
	int ret=0;
	unsigned int i = 0;
	unsigned int slice = 0;
	unsigned int size_per_slice = h->slice_size[0];
	unsigned int expected_bytes = size_per_slice * (unsigned int) nim->nbyper;

	if (h->channels > 0
			|| nim->datatype == NIFTI_TYPE_UINT8 || nim->datatype == NIFTI_TYPE_RGB24 || nim->datatype == NIFTI_TYPE_RGBA32
			|| nim->datatype == NIFTI_TYPE_COMPLEX64 || nim->datatype == NIFTI_TYPE_COMPLEX128 	|| nim->datatype == NIFTI_TYPE_COMPLEX256
			|| nim->datatype == DT_BINARY || nim->datatype == 0) {
		// these types are either not supported or:
		// don't require intensity value adjustments as they are already within the 8bit unsigned range
		h->vol_min_val = 0;
		h->vol_max_val = 255;
		return;
	}

	// initialize min/max
	h->vol_min_val = INFINITY;
	h->vol_max_val = -INFINITY;

	// set time slice to 0 for any data set with dimensionality greater than 3
	if (nim->ndim > 3) dims[4] = 0;

	// we use the first dimension, why not, don't make a difference to me ...
	while (slice < h->sizes[0]) { // SLICE LOOP
		dims[1] = slice;
		if ((ret = nifti_read_collapsed_image(nim, dims, (void*) &data_in))	< 0) {

#ifdef __MINC_NIFTI_CL_CONVERTE__
			fprintf(stderr,"Error reading Nifti data!");
#else
			ERROR("Error reading Nifti data!");
#endif
			return;
		}
		in = NULL;

		//sanity check
		if (ret != expected_bytes) {
#ifdef __MINC_NIFTI_CL_CONVERTE__
			fprintf(stderr,
					"Error reading Nifti data: discrepancy between number of read (%i) and expected bytes (%u)\n", ret, expected_bytes);
#else
			ERROR("Error reading Nifti data: discrepancy between number of read (%i) and expected bytes (%u)!", ret, expected_bytes);
#endif
			if (ret < expected_bytes) {
#ifdef __MINC_NIFTI_CL_CONVERTE__
			fprintf(stderr,	"Conversion aborted!");
#else
			ERROR("Conversion aborted!");
#endif
			return;
			}
		}

		for (i = 0; i < size_per_slice; i++) {
			// move start back "data type" number of bytes...
			if (i == 0)
				in = data_in;
			else
				in = (void *)(((char*) in) + nim->nbyper);

			switch (nim->datatype) {
				case NIFTI_TYPE_INT8: // signed char
					if (((char *) in)[0] < h->vol_min_val) h->vol_min_val = ((char *) in)[0];
					if (((char *) in)[0] > h->vol_max_val) h->vol_max_val = ((char *) in)[0];
					break;
				case NIFTI_TYPE_UINT16: // unsigned short
					if (((unsigned short *) in)[0] < h->vol_min_val) h->vol_min_val = ((unsigned short *) in)[0];
					if (((unsigned short *) in)[0] > h->vol_max_val) h->vol_max_val = ((unsigned short *) in)[0];
					break;
				case NIFTI_TYPE_UINT32: // unsigned int
					if (((unsigned int *) in)[0] < h->vol_min_val) h->vol_min_val = ((unsigned int *) in)[0];
					if (((unsigned int *) in)[0] > h->vol_max_val) h->vol_max_val = ((unsigned int *) in)[0];
					break;
				case NIFTI_TYPE_INT16: // signed short
					if (((short *) in)[0] < h->vol_min_val) h->vol_min_val = ((short *) in)[0];
					if (((short *) in)[0] > h->vol_max_val) h->vol_max_val = ((short *) in)[0];
					break;
				case NIFTI_TYPE_INT32: // signed int
					if (((int *) in)[0] < h->vol_min_val) h->vol_min_val = ((int *) in)[0];
					if (((int *) in)[0] > h->vol_max_val) h->vol_max_val = ((int *) in)[0];
					break;
				case NIFTI_TYPE_UINT64: // unsigned long long
					if (((unsigned long long int *) in)[0] < h->vol_min_val) h->vol_min_val = ((unsigned long long int *) in)[0];
					if (((unsigned long long int *) in)[0] > h->vol_max_val) h->vol_max_val = ((unsigned long long int *) in)[0];
					break;
				case NIFTI_TYPE_INT64: // signed long long
					if (((long long int *) in)[0] < h->vol_min_val) h->vol_min_val = ((long long int *) in)[0];
					if (((long long int *) in)[0] > h->vol_max_val) h->vol_max_val = ((long long int *) in)[0];
					break;
				case NIFTI_TYPE_FLOAT32: //	float
					if (((float *) in)[0] < h->vol_min_val) h->vol_min_val = ((float *) in)[0];
					if (((float *) in)[0] > h->vol_max_val) h->vol_max_val = ((float *) in)[0];
					break;
				case NIFTI_TYPE_FLOAT64: //	double
					if (((double *) in)[0] < h->vol_min_val) h->vol_min_val = ((double *) in)[0];
					if (((double *) in)[0] > h->vol_max_val) h->vol_max_val = ((double *) in)[0];
					break;
				case NIFTI_TYPE_FLOAT128: // long double
					if (((long double *) in)[0] < h->vol_min_val) h->vol_min_val = ((long double *) in)[0];
					if (((long double *) in)[0] > h->vol_max_val) h->vol_max_val = ((long double *) in)[0];
					break;
			}
		}

		// increment slice
		slice++;
		if (data_in != NULL) {
			free(data_in);
			data_in = NULL;
		}
	}
}

void iter_all_pix_and_convert(void *in, unsigned char * out, unsigned int size,	nifti_image *nim, t_header * h, int rgb_channel) {
	unsigned int i = 0;
	unsigned short error = 0;

	for (i = 0; i < size; i++) {
		// keep track of error
		error = 0;
		// move start back "data type" number of bytes...
		if (i != 0)
			in = (void *) (((unsigned char*) in) + nim->nbyper);
		// range to map to
		long double range = h->vol_max_val - h->vol_min_val;
		unsigned char val = 0;

		// now extract value
		switch (nim->datatype) {
			case NIFTI_TYPE_UINT8: // unsigned char | nothing much to there but copy values
				val = (unsigned char) ((unsigned char *) in)[0];
				break;
			case NIFTI_TYPE_INT8: // signed char | adjust range to min/max found previously
				val = (unsigned char)
						roundl(((((long double) ((char *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_UINT16: // unsigned short
				val = (unsigned char)
						roundl(((((long double) ((unsigned short *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_UINT32: // unsigned int
				val = (unsigned char)
						roundl(((((long double) ((unsigned int *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_INT16: // signed short
				val = (unsigned char)
						roundl(((((long double) ((short *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_INT32: // signed int
				val = (unsigned char)
						roundl(((((long double) ((int *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_UINT64: // unsigned long long
				val = (unsigned char)
						roundl(((((long double) ((unsigned long long int *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_INT64: // signed long long
				val = (unsigned char)
						roundl(((((long double) ((long long int *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_FLOAT32: //	float
				val = (unsigned char)
						roundl(((((long double) ((float *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_FLOAT64: //	double
				val = (unsigned char)
						roundl(((((long double) ((double *) in)[0]) - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_FLOAT128: // long double
				val = (unsigned char)
						roundl(((((long double *) in)[0] - h->vol_min_val) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_RGB24:
			case NIFTI_TYPE_RGBA32: // unsigned char per channel
				val = (unsigned char) ((unsigned char *) in)[0];
				out[i * 3 + 0] = val;
				out[i * 3 + 1] = (unsigned char) ((unsigned char *) in)[1];
				out[i * 3 + 2] = (unsigned char) ((unsigned char *) in)[2];
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
		// set value in out array, depending on whether we have rgb or not
		if (h->channels == 0) {
			if (nim->datatype != NIFTI_TYPE_RGB24 && nim->datatype != NIFTI_TYPE_RGBA32)
				out[i * 3 + 0] = out[i * 3 + 1] = out[i * 3 + 2] = val;
		} else out[i * 3 + rgb_channel] = val;

		// check for error
		if (error) {
			// free and good bye
			free(out);
			out = NULL;
		}
	}
}

t_header *create_header_from_nifti_struct(nifti_image *nifti_volume) {
	t_header *h;
	int i;
	int j;

	h = malloc(sizeof(*h));
	h->dim_nb = nifti_volume->ndim;
	if (h->dim_nb > 3) { // this is how we handle time series data and potential RGB, data types in headers are not reliable
		if (h->dim_nb > 4) h->channels = nifti_volume->dim[5]; // if there is yet another dimension after time we assume RGB
		else h->channels = 0;
		h->dim_nb = 3; // internally we use this to determine our loop length
	} else h->channels = 0;;

	h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
	h->sizes_isotropic = malloc(h->dim_nb * sizeof(*h->sizes_isotropic));
	h->start = malloc(h->dim_nb * sizeof(*h->start));
	h->steps = malloc(h->dim_nb * sizeof(*h->steps));
	h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
	h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
	h->dim_offset_isotropic = malloc(h->dim_nb * sizeof(*h->dim_offset_isotropic));
	h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));
	h->slice_size_isotropic = malloc(
			h->dim_nb * sizeof(*h->slice_size_isotropic));

	h->slice_max = (
			nifti_volume->dim[1] * nifti_volume->dim[2]
					> nifti_volume->dim[2] * nifti_volume->dim[3] ?
					(nifti_volume->dim[1] * nifti_volume->dim[2]
							> nifti_volume->dim[1] * nifti_volume->dim[3] ?
							nifti_volume->dim[1] * nifti_volume->dim[2] :
							nifti_volume->dim[1] * nifti_volume->dim[3]) :
					(nifti_volume->dim[2] * nifti_volume->dim[3]
							> nifti_volume->dim[1] * nifti_volume->dim[3] ?
							nifti_volume->dim[2] * nifti_volume->dim[3] :
							nifti_volume->dim[1] * nifti_volume->dim[3]));

	i = 0;
	while (i < h->dim_nb) {
		h->sizes[i] = nifti_volume->dim[i + 1];
		h->sizes_isotropic[i] = h->sizes[i]; // copy over for now and correct it below (if anisotropic)
		h->start[i] = nifti_volume->sto_xyz.m[i][3];
		h->steps[i] = nifti_volume->pixdim[i + 1];
		h->dim_name[i] = strdup("xspace");
		h->dim_name[i][0] = 'x' + i;

		h->slice_size[i] = 1;
		j = 1;
		while (j < h->dim_nb + 1) {
			if ((j - 1) != i) {
				h->slice_size[i] *= nifti_volume->dim[j];
				// copy over for now and correct it below (if anisotropic)
				h->slice_size_isotropic[i] = h->slice_size[i];
			}
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

void conversion_failed_actions(t_args_plug * a, char *id, void * data_out, void * data_in, char * dim_name_char) {
	if (data_in != NULL) {
		free(data_in);
		data_in = NULL;
	}
	if (data_out != NULL) {
		free(data_out);
		data_out = NULL;
	}
	if (dim_name_char != NULL)
		free(dim_name_char);

#ifdef __MINC_NIFTI_CL_CONVERTE__
	fprintf(stderr,"Could not convert slice!\n");
#else
	//a->general_info->percent_cancel(id, a->general_info);
	ERROR("Could not convert slice");
#endif
}

void convertNifti0(t_args_plug *a, nifti_image *nim, t_header *h, int fd, int i,
		int slice_resume, char *id_percent) {
	int j = 0;
	int slice = 0;
	int ret;
	int dims[8] = { 0, -1, -1, -1, -1, -1, -1, -1 };
	void *data_in = NULL;
	unsigned char *data_out = NULL;
	Image *img = NULL;
	PixelPacket *pixels;
	char *dim_name_char = NULL;
	int width = 0;
	int height = 0;
	unsigned int nslices;
	unsigned int size_per_slice;
	int cancel = 0;
	unsigned int expected_bytes = 0;
	int rgb_channel = 0;
	int rgb_total = h->channels > 0 ? h->channels : 1;

#ifdef __MINC_NIFTI_CL_CONVERTE__
				printf("Determining Data Set Range...");
#else
				INFO("Determining Data Set Range...");
#endif
	// find out min/max for range conversion later on
	setGlobalMinMax(nim, h);
	// print out min + max
#ifdef __MINC_NIFTI_CL_CONVERTE__
				printf("[%Lf,%Lf]\n", h->vol_min_val, h->vol_max_val);
#else
				INFO("Data Set Range: [%Lf,%Lf]", h->vol_min_val, h->vol_max_val);
#endif


	dim_name_char = malloc(h->dim_nb * sizeof(*dim_name_char));
	for (j = 0; j < h->dim_nb; j++)
		dim_name_char[j] = h->dim_name[j][0];

	// set time slice to 0 for any data set with dimensionality greater than 3
	if (nim->ndim > 3) dims[4] = 0;
	while (i <= h->dim_nb && cancel == 0) {	// DIMENSION LOOP
		// reset slice or resume
		if (slice_resume > 0) {
			slice = slice_resume;
			slice_resume = -1;
		} else {
			slice = 0;
		}
		nslices = h->sizes[i - 1];
		size_per_slice = h->slice_size[i - 1];
		expected_bytes = size_per_slice * (unsigned int) nim->nbyper;

		data_out = malloc(size_per_slice * 3 * sizeof(*data_out));
		while (slice < nslices && cancel == 0) { // SLICE LOOP
			img = NULL;
			dims[i] = slice;
			rgb_channel = 0;

			while (rgb_channel < rgb_total) { // this loop is done once only for any data type other than RGB
				if (rgb_channel > 3) break; // this means that we ignore the alpha channel
				if (h->channels > 0) dims[nim->ndim] = rgb_channel;

				if ((ret = nifti_read_collapsed_image(nim, dims, (void*) &data_in))	< 0) {
					if (dim_name_char != NULL)
						free(dim_name_char);

				#ifdef __MINC_NIFTI_CL_CONVERTE__
					fprintf(stderr,"Error reading Nifti data!\n");
				#else
					ERROR("Error reading Nifti data!");
				#endif

					return;
				}

				//sanity check
				if (ret != expected_bytes) {
				#ifdef __MINC_NIFTI_CL_CONVERTE__
					fprintf(stderr,
							"Error reading Nifti data: discrepancy between number of read (%i) and expected bytes (%u)\n", ret, expected_bytes);
				#else
					ERROR("Error reading Nifti data: discrepancy between number of read (%i) and expected bytes (%u)!", ret, expected_bytes);
				#endif
					if (ret < expected_bytes) {
				#ifdef __MINC_NIFTI_CL_CONVERTE__
					fprintf(stderr,	"Conversion aborted!");
				#else
					ERROR("Conversion aborted!");
				#endif
					return;
					}
				}

				// loop over values
				iter_all_pix_and_convert(data_in, data_out, size_per_slice, nim, h, rgb_channel);

				if (data_out == NULL) {
					conversion_failed_actions(NULL, NULL, NULL, data_in, dim_name_char);
					return;
				}

				// free data in
				if (data_in != NULL) {
					free(data_in);
					data_in = NULL;
				}
				rgb_channel++;
			}

			get_width_height(&height, &width, i - 1, h->dim_nb, dim_name_char,
					h->sizes);
			img = extractSliceDataAtProperOrientation(NIFTI, RGB_24BIT,
					dim_name_char, i - 1, data_out, width, height, NULL);

			if (img == NULL) {
				conversion_failed_actions(NULL, NULL, data_out, data_in, dim_name_char);
				return;
			}

			// extract pixel info, looping over values
			pixels = GetImagePixels(img, 0, 0, width, height);
			if (pixels == NULL) {
				if (img != NULL)
					DestroyImage(img);
				conversion_failed_actions(NULL, NULL, data_out, data_in, dim_name_char);
				return;
			}

			// sync with data
			for (j = 0; j < size_per_slice; j++) {
				// graphicsmagic quantum depth correction
				if (QuantumDepth != 8 && img->depth == QuantumDepth) {
					data_out[j * 3 + 0] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[j].red);
					data_out[j * 3 + 1] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[j].green);
					data_out[j * 3 + 2] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[j].blue);
					continue;
				} // no correction needed
				data_out[j * 3 + 0] = (unsigned char) pixels[j].red;
				data_out[j * 3 + 1] = (unsigned char) pixels[j].green;
				data_out[j * 3 + 2] = (unsigned char) pixels[j].blue;
			}

			write(fd, data_out, size_per_slice * 3);
			// increment slice
			slice++;

			// tidy up
			if (img != NULL)
				DestroyImage(img);

		#ifdef __MINC_NIFTI_CL_CONVERTE__
			printf("Slice %i / %u of plane '%c'       \r", slice, nslices, dim_name_char[(i - 1)]);
			fflush(stdout);
		#else
			a->general_info->percent_add(1, id_percent, a->general_info);
			cancel = a->general_info->is_percent_paused_cancel(id_percent,
					a->general_info);
			DEBUG("Slice n %i on dimension %i slicenb = %u -- cancel = %i",
					slice, (i - 1), nslices, cancel);
		#endif
		}

		// tidy up
		if (data_out != NULL) {
			free(data_out);
			data_out = NULL;
		}
		dims[i] = -1;
		i++;
	}

	// tidy up
	if (dim_name_char != NULL)
		free(dim_name_char);

#ifdef __MINC_NIFTI_CL_CONVERTE__
	printf("\nConversion finished!\n");
#else
	if (cancel == 0) {
		INFO("Conversion: NIFTI: %s to RAW: %s ==> DONE", a->commands[0], a->commands[1]);
	} else {
		INFO("Conversion: NIFTI: %s to RAW: %s ==> CANCELED", a->commands[0], a->commands[1]);
	}
#endif
}

extern t_log_plugin log_plugin;

#endif /* __NIFTI_CONVERTER__ */
