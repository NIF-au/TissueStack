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

int main(int ac, char **av) {

	int len =0;
	char ext[3];
	int fd = 0;
	enum RAW_FORMAT format;

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

	// in case we crash or abort
	install_signal_manager(av[2], fd);

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

	write_header_into_file(fd, header);
    InitializeMagick("./");
	dim_loop(fd, minc_volume->dim_nb, minc_volume, NULL, NULL, -1, -1);
    DestroyMagick();
}

void	convertNifti(char * nifti_path, int fd) {
	nifti_image		*nim;
	t_header		*h;

	// read nifti for header info
	if ((nim = nifti_image_read(nifti_path, 0)) == NULL) {
		fprintf(stderr,"Error Nifti read");
	    return ;
	}
	h = create_header_from_nifti_struct(nim);

	// write header
	write_header_into_file(fd, h);

	// delegate for rest
    InitializeMagick("./");
	convertNifti0(NULL, nim, h, fd, 1, -1, NULL);
    DestroyMagick();

	/*
	int				i = 1, j=0;
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

	// read nifti to create header info
	if ((nim = nifti_image_read(nifti_path, 0)) == NULL) {
		fprintf(stderr,"Error Nifti read");
	    return ;
	}
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
			data_in = NULL;
			data_out = NULL;
			dims[i] = slice;
			if ((ret = nifti_read_collapsed_image(nim, dims, (void*) &data_in))	< 0) {
				fprintf(stderr,"Error Nifti Get Hyperslab");
				if (dim_name_char != NULL)
					free(dim_name_char);
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
			write(fd, data_out, size_per_slice * 3);

			if (img != NULL)
				DestroyImage(img);
			free(data_out);

			slice++;
			printf("Slice n %i of %i [dimension %i]\r",	slice, nslices, (i - 1));
			fflush(stdout);
		}
		dims[i] = -1;
		i++;
	}
	if (dim_name_char != NULL) free(dim_name_char);
    DestroyMagick();
    */
}
