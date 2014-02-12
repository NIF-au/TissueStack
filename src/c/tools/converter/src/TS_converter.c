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

#include "TS_converter.h"

unsigned int get_slices_max(t_vol *volume) {
	// get the larger number of slices possible
	if ((volume->size[X] * volume->size[Y])
			> (volume->size[Z] * volume->size[X])) {
		if ((volume->size[X] * volume->size[Y])
				> (volume->size[Z] * volume->size[Y]))
			return (volume->size[X] * volume->size[Y]);
	} else if ((volume->size[Z] * volume->size[X])
			> (volume->size[Z] * volume->size[Y]))
		return (volume->size[Z] * volume->size[X]);
	return (volume->size[Z] * volume->size[Y]);
}

void dim_loop(int fd, int dimensions_nb, t_vol *volume, t_tissue_stack *t,
		char *id_percent, int slice_resume, int dimension_resume) {
	int dim = 0;
	int slice = 0;
	int this_slice = 0;
	int size;
	unsigned char *hyperslab = NULL;
	int i = 0, j = 0;
	unsigned long *start;
	long unsigned int *count;
	short cancel = 0;
	char *dim_name_char = NULL;
	Image *img = NULL;
	PixelPacket *pixels;
	unsigned long long int pixel_value = 0;
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

	printf(
			"Dimensions size: size[0] ==> %i | size[1] ==> %i | size[2] ==> %i\n\n",
			(int) volume->size[0], (int) volume->size[1],
			(int) volume->size[2]);
	while (dim < dimensions_nb && cancel == 0) { // DIMENSION LOOP
		size = (dim == 0 ?
				(volume->size[2] * volume->size[1]) :
				(dim == 1 ?
						(volume->size[0] * volume->size[2]) :
						(volume->size[0] * volume->size[1])));
		hyperslab = malloc(size * sizeof(*hyperslab));
		slice = volume->size[dim];
		if (slice_resume != -1) {
			this_slice = slice_resume;
			slice_resume = -1;
		} else
			this_slice = 0;
		count[dim] = 1;
		while (this_slice < slice && cancel == 0) { // SLICE LOOP
			img = NULL;
			start[dim] = this_slice;
			memset(hyperslab, '\0', size);

			// read hyperslab
			miget_real_value_hyperslab(volume->minc_volume, MI_TYPE_UBYTE,
					(misize_t*) start, (misize_t*) count, hyperslab);

			// convert to image and perform proper orientation corrections
			get_width_height(&height, &width, dim, volume->dim_nb,
					dim_name_char, volume->size);
			volume->original_format = MINC;
			img = extractSliceDataAtProperOrientation(volume->original_format,
					dim_name_char, dim, hyperslab, width, height, NULL);
			if (img == NULL) {
				ERROR("Could not convert slice");
				free(hyperslab);
				return;
			}

			// extract pixel info, looping over values
			pixels = GetImagePixels(img, 0, 0, width, height);
			if (pixels == NULL) {
				ERROR("Could not convert slice");
				if (img != NULL)
					DestroyImage(img);
				free(hyperslab);
				return;
			}
			for (j = 0; j < size; j++) {
				pixel_value = pixels[(width * i) + j].red;
				if (QuantumDepth != 8 && img->depth == QuantumDepth)
					pixel_value = mapUnsignedValue(img->depth, 8, pixel_value);
				hyperslab[j] = (char) pixel_value;
			}
			write(fd, hyperslab, size);

			printf("Slice = %i / %i - dim = %i\r", this_slice,
					(int) volume->size[dim], dim);
			fflush(stdout);
			this_slice++;
			if (img != NULL)
				DestroyImage(img);

		}
		start[dim] = 0;
		count[dim] = volume->size[dim];
		dim++;
		free(hyperslab);
		printf(
				"                                                                                                                                          \r");
	}
	if (dim_name_char != NULL)
		free(dim_name_char);
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

int main(int ac, char **av) {

	int len =0;
	char ext[3];
	int fd = 0;
	enum FORMAT format;

	if (ac < 3) {
		printf("Usage: ./mncOrNii2raw  [minc/nifti volume path] [raw volume path]\n");
		return (-1);
	}

	// distinguish between mnc and nifti
	len = strlen(av[1]);
	if (len < 4) {
		fprintf(stderr, "File Name must have an extension that is either .mnc or .nii to distinguish between the two!\n");
		return -1;
	}

	memset(ext, 0, 3);
	memcpy(ext, &av[1][len-3], 3);
	if (strcmp(ext, "nii") == 0) format = NIFTI;
	else if (strcmp(ext, "mnc") == 0) format = MINC;
	else {
		fprintf(stderr, "File Name must be either minc/infti with respective extension .mnc or .nii!\n");
		return -1;
	}

	// create raw file to write to
	if ((fd = open(av[2], (O_CREAT | O_TRUNC | O_RDWR), 0666)) == -1) {
		fprintf(stderr, "Open Failed\n");
		return (-1);
	}
	if (chmod(av[2], 0644) == -1)
		fprintf(stderr, "Chmod failed\n");

	// delegate to subroutines for conversion
	if (format == NIFTI) convertNifti(av[1], fd);
	else if (format == MINC) convertMinc(av[1], fd);

	// close file descriptor
	if (fd != 0) close(fd);

	return (0);
}

void	convertMinc(char * minc_path, int fd) {
	t_vol *minc_volume;
	t_header *header;

	minc_volume = init_get_volume_from_minc_file(minc_path);
	if (minc_volume == NULL) return;

	header = create_header_from_minc_struct(minc_volume);

    InitializeMagick("./");
	write_header_into_file(fd, header);
	dim_loop(fd, minc_volume->dim_nb, minc_volume, NULL, NULL, -1, -1);
    DestroyMagick();
}

void	convertNifti(char * nifti_path, int fd) {
	nifti_image		*nim;
	t_header		*h;
	int				i = 1, j=0;
	int				slice = 0;
	int				ret;
	int				sizes[3];
	int				dims[8] = { 0,  -1, -1, -1, -1, -1, -1, -1 };
	unsigned char	*data = NULL;
	unsigned char	*data_char;
	Image			*img = NULL;
	PixelPacket 	*pixels;
	char 			*dim_name_char = NULL;
	int 			width = 0;
	int 			height = 0;
	int				nslices;
	unsigned int	size_per_slice;

	unsigned long long int pixel_value = 0;

	// read nifti for header info
	if ((nim = nifti_image_read(nifti_path, 0)) == NULL) {
		fprintf(stderr,"Error Nifti read");
	    return ;
	}

	sizes[0] = nim->dim[1];
	sizes[1] = nim->dim[2];
	sizes[2] = nim->dim[3];

	h = create_header_from_nifti_struct(nim);

	// write header
	write_header_into_file(fd, h);

	dim_name_char = malloc(h->dim_nb * sizeof(*dim_name_char));
	for (j=0;j<h->dim_nb;j++)
		dim_name_char[j] = h->dim_name[j][0];

    InitializeMagick("./");

	while (i <= nim->dim[0]) {	// DIMENSION LOOP
		slice = 0;
		nslices = sizes[i - 1];
		size_per_slice = h->slice_size[i - 1];
		while (slice < nslices) { // SLICE LOOP
			img = NULL;
			data = NULL;
			dims[i] = slice;
			if ((ret = nifti_read_collapsed_image(nim, dims, (void*) &data))
					< 0) {
				fprintf(stderr,"Error Nifti Get Hyperslab");
				if (dim_name_char != NULL)
					free(dim_name_char);
				return;
			}
			if (ret > 0) {
				data_char = iter_all_pix_and_convert(data, size_per_slice, nim);

				get_width_height(&height, &width, i - 1, h->dim_nb,
						dim_name_char, h->sizes);
				img = extractSliceDataAtProperOrientation(NIFTI, dim_name_char,
						i - 1, data_char, width, height, NULL);
				if (img == NULL) {
					fprintf(stderr,"Could not convert slice");
					free(data_char);
					if (dim_name_char != NULL)
						free(dim_name_char);
					return;
				}

				// extract pixel info, looping over values
				pixels = GetImagePixels(img, 0, 0, width, height);
				if (pixels == NULL) {
					fprintf(stderr,"Could not convert slice");
					if (img != NULL)
						DestroyImage(img);
					free(data_char);
					if (dim_name_char != NULL)
						free(dim_name_char);
					return;
				}
				for (j = 0; j < size_per_slice; j++) {
					pixel_value = pixels[(width * i) + j].red;
					if (QuantumDepth != 8 && img->depth == QuantumDepth)
						pixel_value = mapUnsignedValue(img->depth, 8,
								pixel_value);
					data_char[j] = (char) pixel_value;
				}
				write(fd, data_char, size_per_slice);

				if (img != NULL)
					DestroyImage(img);
				free(data_char);
			}
			free(data);
			slice++;
			printf("Slice n %i of %i [dimension %i]\r",	slice, nslices, (i - 1));
			fflush(stdout);
		}
		dims[i] = -1;
		i++;
	}
	if (dim_name_char != NULL) free(dim_name_char);
    DestroyMagick();
}
