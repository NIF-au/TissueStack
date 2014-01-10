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
	t_vol			*volume;

	a = (t_args_plug *)args;
	prctl(PR_SET_NAME, "TS_QUERY");

	socketDescriptor = (FILE*)a->box;
	volume = load_volume(a, a->commands[0]);
	if (volume == NULL) {
		write_http_header(socketDescriptor, "500 Server Error", "png");
		fclose(socketDescriptor);
		return NULL;
	}

	INFO("Number of dims: %i", volume->dim_nb);
	// loop over them
	int x=0;
	while (x<volume->dim_nb) {
		INFO("Dim[%i] => %c", x,volume->dim_name_char[x]);
		INFO("Length => %llu", volume->dim_offset[x]);
		INFO("Size => %u", volume->size[x]);
		INFO("Starts => %f", volume->starts[x]);
		INFO("Stepd => %f", volume->steps[x]);
		x++;
	}

	write_http_header(socketDescriptor, "200 OK", "png");
	fclose(socketDescriptor);

	return NULL;
}

void		*unload(void *args) {
	INFO("Image Query Plugin: Unloaded");
	return (NULL);
}
