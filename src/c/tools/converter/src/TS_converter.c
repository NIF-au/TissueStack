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
	if (strcmp(ext, "nii") == 0 || strcmp(ext, ".gz") == 0) format = NIFTI;
	else if (strcmp(ext, "mnc") == 0) format = MINC;
	else {
		fprintf(stderr, "File Name must be either minc/nifti with respective extension .mnc or .nii/.nii.gz!\n");
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
	dim_loop(fd, header->dim_nb, minc_volume, NULL, NULL, -1, -1, NULL, header);
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
}
