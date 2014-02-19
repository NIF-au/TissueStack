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
#include "nifti_converter.h"

void *init(void *args) {
	t_args_plug *a = (t_args_plug *) args;

	LOG_INIT(a);
	InitializeMagick("./");
	INFO("Nifti Converter initialized.");

	return (NULL);
}

void *start(void *args) {
	int dims[8] = { 0, -1, -1, -1, -1, -1, -1, -1 };
	int sizes[3];
	void *data_in = NULL;
	unsigned char *data_out = NULL;
	int slice = 0;
	int ret;
	nifti_image *nim;
	int nslices;
	int i = 0, j = 0;
	int fd = 0;
	unsigned int size_per_slice;
	t_header *h;
	t_args_plug *a;
	char *id_percent;
	int cancel = 0;
	unsigned int dimensions_resume = -1;
	unsigned int slice_resume = -1;
	unsigned long long off = 0L;
	char *command_line = NULL;
	Image *img = NULL;
	PixelPacket * pixels;
	char * dim_name_char = NULL;
	int width = 0;
	int height = 0;

	prctl(PR_SET_NAME, "TS_NIFTI_CON");

	a = (t_args_plug*) args;
	if ((nim = nifti_image_read(a->commands[0], 0)) == NULL) {
		ERROR("Error Nifti read");
		return (NULL);
	}

	sizes[0] = nim->dim[1];
	sizes[1] = nim->dim[2];
	sizes[2] = nim->dim[3];

	h = create_header_from_nifti_struct(nim);

	if ((a->commands[3] != NULL && a->commands[4] != NULL
			&& a->commands[5] != NULL)
			|| (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0
					&& a->commands[3] != NULL && strlen(a->commands[3]) == 16)) {
		if (a->commands[2] != NULL) {
			if (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0
					&& a->commands[3] != NULL && strlen(a->commands[3]) == 16) {
				id_percent = a->commands[3];
				dimensions_resume = 0;
				slice_resume = 0;
			} else {
				id_percent = a->commands[5];
				dimensions_resume = atoi(a->commands[2]);
				slice_resume = atoi(a->commands[3]);
			}
			if ((fd = open(a->commands[1], (O_CREAT | O_APPEND | O_RDWR), 0666))
					== -1) {
				ERROR("Open Failed");
				return (NULL);
			}
			i = 0;
			while (i < dimensions_resume) {
				off += h->dim_offset[i];
				i++;
			}
			if (slice_resume != 0)
				off += h->slice_size[i] * (slice_resume - 1);
			lseek(fd, off, SEEK_SET);
			i = dimensions_resume + 1;
		}
	} else {
		if ((fd = open(a->commands[1], O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0) {
			ERROR("Open error");
			return (NULL);
		}
		if (chmod(a->commands[1], 0644) == -1)
			ERROR("Chmod Error");
		write_header_into_file(fd, h);
		if (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0) {
			command_line = array_2D_to_array_1D(a->commands);
			a->general_info->percent_init((sizes[0] + sizes[1] + sizes[2]),
					&id_percent, a->commands[0], "2", a->commands[1],
					command_line, a->general_info);
			if (a->box != NULL) {
				if (write(*((int*) a->box), id_percent, 16) < 0)
					ERROR("Open Error");
			}
			return (NULL);
		}
		i = 1;
	}

	dim_name_char = malloc(h->dim_nb * sizeof(*dim_name_char));
	for (j = 0; j < h->dim_nb; j++)
		dim_name_char[j] = h->dim_name[j][0];

	while (i <= nim->dim[0] && cancel == 0) {
		if (slice_resume != -1) {
			slice = slice_resume;
			slice_resume = -1;
		} else
			slice = 0;

		nslices = sizes[i - 1];
		size_per_slice = h->slice_size[i - 1];
		while (slice < nslices && cancel == 0) {
			img = NULL;
			data_in = NULL;
			data_out = NULL;
			dims[i] = slice;

			if ((ret = nifti_read_collapsed_image(nim, dims, (void*) &data_in))	< 0) {
				ERROR("Error Nifti Get Hyperslab");
				if (dim_name_char != NULL) free(dim_name_char);
				return (NULL);
			}

			// read nifti data and convert it to proper data type
			data_out = iter_all_pix_and_convert(data_in, size_per_slice, nim);
			free(data_in);
			if (data_out == NULL) {
				conversion_failed_actions(a, id_percent, data_out,	dim_name_char);
				return NULL;
			}

			get_width_height(&height, &width, i - 1, h->dim_nb, dim_name_char,	h->sizes);
			img = extractSliceDataAtProperOrientation(NIFTI, RGB_24BIT,	dim_name_char, i - 1, data_out, width, height, NULL);
			if (img == NULL) {
				conversion_failed_actions(a, id_percent, data_out,	dim_name_char);
				return NULL;
			}

			// extract pixel info, looping over values
			pixels = GetImagePixels(img, 0, 0, width, height);
			if (pixels == NULL) {
				if (img != NULL)
					DestroyImage(img);
				conversion_failed_actions(a, id_percent, data_out,
						dim_name_char);
				return NULL;
			}

			// sync with data
			for (j = 0; j < size_per_slice; j++) {
				// graphicsmagic quantum depth correction
				if (QuantumDepth != 8 && img->depth == QuantumDepth) {
					data_out[j * 3 + 0] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].red);
					data_out[j * 3 + 1] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].green);
					data_out[j * 3 + 2] = (unsigned char) mapUnsignedValue(img->depth, 8, pixels[(width * i) + j].blue);
					continue;
				} // no correction needed
				data_out[j * 3 + 0] = (unsigned char) pixels[(width * i) + j].red;
				data_out[j * 3 + 1] = (unsigned char) pixels[(width * i) + j].green;
				data_out[j * 3 + 2] = (unsigned char) pixels[(width * i) + j].blue;
			}
			write(fd, data_out, size_per_slice * 3);

			if (img != NULL) DestroyImage(img);
			free(data_out);
			slice++;
			a->general_info->percent_add(1, id_percent, a->general_info);
			cancel = a->general_info->is_percent_paused_cancel(id_percent,	a->general_info);
			DEBUG("Slice n %i on dimension %i slicenb = %i -- cancel = %i",	slice, (i - 1), nslices, cancel);
		}
		dims[i] = -1;
		i++;
	}
	if (dim_name_char != NULL)	free(dim_name_char);

	if (cancel == 0)
		INFO("Conversion: NIFTI: %s to RAW: %s ==> DONE", a->commands[0], a->commands[1]);

	close(fd);
	a->destroy(a);

	return (NULL);
}

void *unload(void *args) {
	DestroyMagick();
	INFO("Nifti Converter Plugin: Unloaded");
	return (NULL);
}

void conversion_failed_actions(t_args_plug * a, char *id, void * data_out, char * dim_name_char) {
	a->general_info->percent_cancel(id, a->general_info);

	if (data_out != NULL) free(data_out);
	if (dim_name_char != NULL) free(dim_name_char);

	ERROR("Could not convert slice");
}
