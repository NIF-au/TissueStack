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
	t_args_plug *a;
	nifti_image *nim;
	t_header *h;
	int fd = 0;
	int i = 0;
	int sizes[3];
	char *id_percent;
	unsigned int dimensions_resume = -1;
	unsigned int slice_resume = -1;
	unsigned long long offset = 0L;
	char *command_line = NULL;

	prctl(PR_SET_NAME, "TS_NIFTI_CON");
	a = (t_args_plug*) args;

	// read nifti
	if ((nim = nifti_image_read(a->commands[0], 0)) == NULL) {
		ERROR("Error Nifti read");
		return (NULL);
	}

	sizes[0] = nim->dim[1];
	sizes[1] = nim->dim[2];
	sizes[2] = nim->dim[3];

	// create header
	h = create_header_from_nifti_struct(nim);

	if ((a->commands[3] != NULL && a->commands[4] != NULL
			&& a->commands[5] != NULL)
			|| (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0
					&& a->commands[3] != NULL && strlen(a->commands[3]) == 16)) {
		if (a->commands[2] != NULL) {
			if (a->commands[2] != NULL && strcmp(a->commands[2], "@tasks@") == 0
					&& a->commands[3] != NULL && strlen(a->commands[3]) == 16) {
				INFO("First in");
				id_percent = a->commands[3];
				dimensions_resume = 0;
				slice_resume = 0;
			} else {
				INFO("second in");
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
				offset += h->dim_offset[i];
				i++;
			}
			if (slice_resume > 0) offset += (h->slice_size[i] * slice_resume * 3);
			lseek(fd, offset, SEEK_SET);
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
			INFO("Third in");
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

	// delegate
	convertNifti0(a, nim, h, fd, i, slice_resume, id_percent);

	// close file and clean up
	close(fd);
	a->destroy(a);

	return (NULL);
}

void *unload(void *args) {
	DestroyMagick();
	INFO("Nifti Converter Plugin: Unloaded");
	return (NULL);
}
