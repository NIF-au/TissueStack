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
#include "minc_info.h"

void *init(void *args) {
  // nothing to do
  return NULL;
}

void *start(void *args) {
	if (args == NULL) {
		return NULL;
	}

	t_args_plug	*a = (t_args_plug *)args;
	// check if commands is null, we expect the filename!
	if (a->commands == NULL) {
		return NULL;
	}

    prctl(PR_SET_NAME, "TS_MINC_INFO");

	// get our unix socket
	int * unix_socket = (int *)a->box;

	// get volume info
	t_vol *volume = a->general_info->check_volume(a->commands[0], a->general_info);

	// string buffer
	t_string_buffer * buffer = NULL;

	// start 'amateurish' serialization of data as a CSV string

	// if volume is null send a simple "NULL"
	if (volume == NULL) {
		buffer = appendToBuffer(buffer, "NULL");
	} else {
		// start perusing contents and turning them into strings or nulls
		// first filename
		buffer = appendToBuffer(buffer, volume->path == NULL ? "NULL" : volume->path);
		buffer = appendToBuffer(buffer, "|");

		// append number of dimensions
		char * convertedInt = malloc(sizeof(convertedInt) * 25);
		sprintf(convertedInt, "%i", volume->dim_nb);
		buffer = appendToBuffer(buffer, convertedInt);
		buffer = appendToBuffer(buffer, "|");
		free(convertedInt);

		if (volume->dim_name == NULL) {
			buffer = appendToBuffer(buffer, "NULL");
		}
		// dimension names
		int i = 0;
		while (i < volume->dim_nb) {
			buffer = appendToBuffer(buffer, volume->dim_name[i]);
			if (i != (volume->dim_nb -1)) buffer = appendToBuffer(buffer, ":");
			i++;
		}
		buffer = appendToBuffer(buffer, "|");
		//actual dimensions
		if (volume->size == NULL) {
			buffer = appendToBuffer(buffer, "NULL");
		}
		i = 0;
		while (i < volume->dim_nb) {
			char * convertedInt = malloc(sizeof(convertedInt) * 25);
			sprintf(convertedInt, "%u", volume->size[i]);

			buffer = appendToBuffer(buffer, convertedInt);
			if (i != (volume->dim_nb -1)) buffer = appendToBuffer(buffer, ":");

			free(convertedInt);
			i++;
		}
		buffer = appendToBuffer(buffer, "|");
		// steps
		if (volume->steps == NULL) {
			buffer = appendToBuffer(buffer, "NULL");
		}
		i = 0;
		while (i < volume->dim_nb) {
			char * convertedDouble = malloc(sizeof(convertedDouble) * 25);
			sprintf(convertedDouble, "%f", volume->steps[i]);

			buffer = appendToBuffer(buffer, convertedDouble);
			if (i != (volume->dim_nb -1)) buffer = appendToBuffer(buffer, ":");

			free(convertedDouble);
			i++;
		}
		buffer = appendToBuffer(buffer, "|");
		// starts
		if (volume->starts == NULL) {
			buffer = appendToBuffer(buffer, "NULL");
		}
		i = 0;
		while (i < volume->dim_nb) {
			char * convertedDouble = malloc(sizeof(convertedDouble) * 25);
			sprintf(convertedDouble, "%f", volume->starts[i]);

			buffer = appendToBuffer(buffer, convertedDouble);
			if (i != (volume->dim_nb -1)) buffer = appendToBuffer(buffer, ":");

			free(convertedDouble);
			i++;
		}
	}

	DEBUG("Sending: #%s#", buffer->buffer);

	// write out response
	send(*unix_socket, buffer->buffer, buffer->size, 0);

	// close socket
	shutdown(*unix_socket, 2);
	close(*unix_socket);

	return NULL;
}

void *unload(void *args) {
	// nothing to do
	return NULL;
}
