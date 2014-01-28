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

#include "core.h"

void		*init(void *args) {
	  t_args_plug	*a = (t_args_plug *)args;

	  prctl(PR_SET_NAME, "Image_plug");
	  LOG_INIT(a);

	  // free command line args
	  a->destroy(a);

	  INFO("Image Extract Plugin: Started");
	  return (NULL);
}

void			*start(void *args) {
	INFO("Image Query Plugin: Entered");

	t_args_plug		*a;
	FILE			*socketDescriptor;
	int 			i=0;
	t_vol			*volume = NULL;
	unsigned long long int offset = 0;
	char * pixel = NULL;
	int 			is_raw = 0;
	int 			dim=0;
	int 			slice=0;
	int 			x=0;
	int 			y=0;

	a = (t_args_plug *)args;
	prctl(PR_SET_NAME, "TS_QUERY");

	socketDescriptor = (FILE*)a->box;

	// param reads
	while (a->commands != NULL && a->commands[i] != NULL) {
		switch (i) {
			case 0:
				volume = load_volume(a, a->commands[i]);
				break;
			case 1:
				dim = atoi(a->commands[i]);
				break;
			case 2:
				slice = atoi(a->commands[i]);
				break;
			case 3:
				y = atoi(a->commands[i]);
				break;
			case 4:
				x = atoi(a->commands[i]);
				break;
		}
		i++;
	}

	// sanity checks
	if (i != 5) {
		write_http_header2(socketDescriptor, "Query takes inputs: volume, dimension, slice, y and x!", NULL);
		fclose(socketDescriptor);
		return NULL;
	}

	if (volume == NULL) {
		write_http_header2(socketDescriptor, "Volume not found or null", NULL);
		fclose(socketDescriptor);
		return NULL;
	}

	// check if is raw file
	is_raw = israw(volume->path, volume->raw_fd);
	if (is_raw <= 0) {
		write_http_header2(socketDescriptor, "Volume has to be in RAW format to be queried!", NULL);
		fclose(socketDescriptor);
		return NULL;
	}

	INFO("is raw: %i", (int)is_raw);
	INFO("dim: %c", volume->dim_name_char[dim]);
	INFO("slice: %i", slice);
	INFO("x: %i", x);
	INFO("y: %i", y);

	INFO("Size => %u", volume->size[dim]);
	INFO("Starts => %f", volume->starts[dim]);
	INFO("Steps => %f", volume->steps[dim]);

	offset = (volume->dim_offset[dim] + (unsigned long long int)((unsigned long long int)volume->slice_size[dim] * (unsigned long long int)slice));
	INFO("Offset => %llu", offset);
	pixel = malloc(1 * sizeof(*pixel));

	INFO("RAW FD: %i", volume->raw_fd);

	INFO("LSEEK....%i", (int) lseek(volume->raw_fd, offset, SEEK_SET));
	INFO("READ....%i", (int) read(volume->raw_fd, (void *) pixel, 1));
	INFO("PIXEL: %i", (int) pixel[0]);

	write_http_header2(socketDescriptor, pixel, "200 OK");
	fclose(socketDescriptor);

	return NULL;
}

void		*unload(void *args) {
	INFO("Image Query Plugin: Unloaded");
	return (NULL);
}
