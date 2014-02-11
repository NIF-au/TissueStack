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
#include "image_query.h"

void		*init(void *args) {
	  t_args_plug	*a = (t_args_plug *)args;

	  prctl(PR_SET_NAME, "Image_plug");
	  LOG_INIT(a);

	  InitializeMagick("./");

	  // free command line args
	  a->destroy(a);

	  INFO("Image Extract Plugin: Started");
	  return (NULL);
}

void			*start(void *args) {
	t_args_plug				*a;
	FILE					*socketDescriptor;
	int 					i=0;
	t_vol					*volume = NULL;
	unsigned long long int 	offset = 0;
	unsigned int 			combined_size = 0;
	unsigned char 			*image_data = NULL;
	ExceptionInfo			exception;
	Image 					*img = NULL;
	PixelPacket 			px;
	unsigned char 		pixel = '\0';
	unsigned long long 		*pixel_value = NULL;
	int			 			is_raw = 0;
	int 					dim=0;
	int 					slice=0;
	int 					x=0;
	int 					y=0;
	int width = 0;
	int height = 0;
	// since multiple volumes in one request are possible we have to loop through the choices ...
	int 					nr_of_volumes = 1;
	char 					**volumes = NULL;
	char 					**dimensions = NULL;
	char 					**slices = NULL;
	char 					**xes = NULL;
	char 					**ys = NULL;
	t_string_buffer			*response = NULL;
	char value[100];

	a = (t_args_plug *)args;
	prctl(PR_SET_NAME, "TS_QUERY");

	socketDescriptor = (FILE*)a->box;

	// param sanity check
	while (a->commands != NULL && a->commands[i] != NULL) {
		i++;
	}
	if (i != 5) {
		write_http_response(socketDescriptor,
  			  "{\"error\": {\"description\": \"Application Exception\", \"message\": \
  			  \"Query takes inputs: volume, dimension, slice, y and x!\"}}",
  			  NULL, "application/json");
		fclose(socketDescriptor);
		return NULL;
	}
	i = 0;

	// this is for multiple volume queries at once
	if (strstr(a->commands[0], ":") != NULL) {
		nr_of_volumes = countTokens(a->commands[0], ':','\\');
		volumes =  tokenizeString(a->commands[0], ':','\\');
	  	dimensions = tokenizeString(a->commands[1], ':','\\');
	  	slices = tokenizeString(a->commands[2], ':','\\');
	  	xes = tokenizeString(a->commands[4], ':','\\');
	  	ys = tokenizeString(a->commands[3], ':','\\');
	  }  else { // old fashioned single volume request to have 1 loop only
		volumes = malloc(2*sizeof(*volumes));
		volumes[0] = a->commands[0];
		volumes[1] = NULL;
		dimensions = malloc(2*sizeof(*dimensions));
		dimensions[0] = strdup(a->commands[1]);
		dimensions[1] = NULL;
		slices = malloc(2*sizeof(*slices));
		slices[0] = a->commands[2];
		slices[1] = NULL;
		xes = malloc(2*sizeof(*xes));
		xes[0] = a->commands[4];
		xes[1] = NULL;
		ys = malloc(2*sizeof(*ys));
		ys[0] = a->commands[3];
		ys[1] = NULL;
	  }

	// start response
	response = appendToBuffer(response, "{\"response\": {");

	// loop over all volumes handed in
	while (i<nr_of_volumes) {
		if (volumes[i] == NULL) break;

		volume = load_volume(a, volumes[i]);

		if (volume == NULL) {
			writeError(socketDescriptor, "Volume not found or null!", volumes, dimensions, slices, xes, ys);
			return NULL;
		}

		// check if is raw file
		is_raw = israw(volume->path, volume->raw_fd);
		if (is_raw <= 0) {
			writeError(socketDescriptor, "Volume has to be in RAW format to be queried!", volumes, dimensions, slices, xes, ys);
			return NULL;
		}

		// add volume to response
		if (i != 0) response = appendToBuffer(response, ", ");
		response = appendToBuffer(response, "\"");
		response = appendToBuffer(response, volume->path);
		response = appendToBuffer(response, "\" : ");

		// get proper query param associated with volume index
		if (dimensions[i] == NULL || slices[i] == NULL || xes[i] == NULL || ys[i] == NULL) {
			writeError(socketDescriptor,
					"In the case of multiple volume queries each parameter has to correspond to the number of volumes queried!",
					volumes, dimensions, slices, xes, ys);
			return NULL;
		}

		if (nr_of_volumes > 1) dimensions[i][0] = get_by_name_dimension_id(volume, dimensions[i]);
		dim = atoi(str_n_cpy(dimensions[i], 0, 1));
		slice = atoi(slices[i]);
		x = atoi(xes[i]);
		y = atoi(ys[i]);

		// find offset and dimensions
		offset = (volume->dim_offset[dim] + (unsigned long long int)((unsigned long long int)volume->slice_size[dim] * (unsigned long long int)slice));
		get_width_height(&height, &width, dim, volume->dim_nb, volume->dim_name_char, volume->size);
		combined_size = width * height;

		//reset
		GetExceptionInfo(&exception);
		img = NULL;
		lseek(volume->raw_fd, 0, SEEK_SET); // return to beginning, just to be safe
		memset(value, 0, 100);

		// if generic the raw is ready to use as is
		if (volume->original_format == GENERIC) {
			lseek(volume->raw_fd, offset + y*width + x, SEEK_SET);
			read(volume->raw_fd, &pixel, 1);

			// set response
			sprintf(value, " {\"red\": %cu, \"green\": %cu, \"blue\": %cu}", pixel, pixel, pixel);
			/*
			if ((img = ConstituteImage(width, height, "I", CharPixel, image_data, &exception)) == NULL) {
			  dealWithException(&exception, NULL, NULL, NULL);
			  if (image_data != NULL) free(image_data);
			  return NULL;
		  }*/
		} else {	// do some orientation nonsense
			image_data = malloc(combined_size*sizeof(*image_data));
			lseek(volume->raw_fd, offset, SEEK_SET);
			read(volume->raw_fd, image_data, combined_size);

			img = extractSliceDataAtProperOrientation(volume->original_format, volume->dim_name_char, dim, image_data, width, height, socketDescriptor);
			if (img == NULL) { // something went wrong => say good bye
				if (image_data != NULL) free(image_data);
				return NULL;
			}

			// fetch pixel value and address GraphicsMagick quantum differences among systems
			px = GetOnePixel(img, x, y);
			pixel_value = malloc(3*sizeof(*pixel_value)); // RGB
			pixel_value[0] = (unsigned long long int) px.red;
			pixel_value[1] = (unsigned long long int) px.green;
			pixel_value[2] = (unsigned long long int) px.blue;
			if (QuantumDepth != 8 && img->depth == QuantumDepth) {
				pixel_value[0] = mapUnsignedValue(img->depth, 8, pixel_value[0]);
				pixel_value[1] = mapUnsignedValue(img->depth, 8, pixel_value[1]);
				pixel_value[2] = mapUnsignedValue(img->depth, 8, pixel_value[2]);
			}

			// set response
			sprintf(value, " {\"red\": %llu, \"green\": %llu, \"blue\": %llu}", pixel_value[0], pixel_value[1], pixel_value[2]);

			// clean up
			if (image_data != NULL) free(image_data);
			if (img != NULL) DestroyImage(img);
			if (pixel_value != NULL) free(pixel_value);
		}

		// add voxel value to response
		response = appendToBuffer(response, value);

		//increment
		i++;
	}

	// finish off json
	response = appendToBuffer(response, " } }");

	// write response into socket
	write_http_response(socketDescriptor, response->buffer, "200 OK", "application/json");
	fclose(socketDescriptor);

	//clean up
	free(response->buffer);
	free(response);
	free_null_terminated_char_2D_array(volumes);
	free_null_terminated_char_2D_array(dimensions);
	free_null_terminated_char_2D_array(slices);
	free_null_terminated_char_2D_array(xes);
	free_null_terminated_char_2D_array(ys);

	return NULL;
}

void		*unload(void *args) {
    DestroyMagick();

	INFO("Image Query Plugin: Unloaded");
	return (NULL);
}

void writeError(FILE * socketDescriptor, char * error, char **volumes, char **dimensions, char **slices, char **xes, char **ys) {
	char text[200];

	// compose error message
	if (error != NULL)
		sprintf(text, "{\"error\": {\"description\": \"Application Exception\", \"message\": \"%s\"}}", error);

	// write out error
	write_http_response(socketDescriptor, text, NULL, "application/json");
	fclose(socketDescriptor);

	// clean up
	free_null_terminated_char_2D_array(volumes);
	free_null_terminated_char_2D_array(dimensions);
	free_null_terminated_char_2D_array(slices);
	free_null_terminated_char_2D_array(xes);
	free_null_terminated_char_2D_array(ys);
}
