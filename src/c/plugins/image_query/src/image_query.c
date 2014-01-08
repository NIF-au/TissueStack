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

void		*init(void *args)
{
  //t_image_extract	*image_args;
  t_args_plug	*a;
  a = (t_args_plug *)args;
  prctl(PR_SET_NAME, "Query_plug");

  LOG_INIT(a);

  INFO("Image Query Plugin: Initialized");

  // free command line args
  a->destroy(a);

  return (NULL);
}

void			*start(void *args)
{
	  INFO("Image Query Plugin: Entered");

	  //t_image_extract	*image_args;
  t_args_plug		*a;
  FILE			*socketDescriptor;
  t_vol			*volume;

  a = (t_args_plug *)args;
  prctl(PR_SET_NAME, "TS_QUERY");

  socketDescriptor = (FILE*)a->box;
  volume = load_volume(a, a->commands[0]);
  if (volume == NULL) {
    write_http_header(socketDescriptor, "500 Server Error", NULL);
    fclose(socketDescriptor);
    return NULL;
  }

  write_http_header(socketDescriptor, "200 OK", NULL);
  fclose(socketDescriptor);

  INFO("Image Query Plugin: Started");

  a->destroy(a);

  return (NULL);
}

void		*unload(void *args)
{
  //t_image_extract	*image_args = NULL;
  t_args_plug	*a = NULL;

  a = (t_args_plug *)args;

  INFO("Image Query Plugin: Unloaded");

  if (a != NULL) {
	  free(a->name);
	  free(a->path);
	  free(a);
	  a = NULL;
  }

  DestroyMagick();

  return (NULL);
}

void			free_image_extract(t_image_extract * extract) {
	if (extract == NULL) {
		return;
	}

	if (extract->dim_start_end != NULL) {
		int i=0;
		while (i<extract->dim_nb) {
			if (extract->dim_start_end[i] != NULL) free(extract->dim_start_end[i]);
			i++;
		}
		free(extract->dim_start_end);
	}

	if (extract->image_type != NULL) free(extract->image_type);
	if (extract->root_path != NULL)  free(extract->root_path);
	if (extract->service != NULL)    free(extract->service);
	if (extract->request_id != NULL)    free(extract->request_id);
	if (extract->request_time != NULL)    free(extract->request_time);

	free(extract);
	extract = NULL;
}

void			free_image_args(t_image_args * args) {
	if (args == NULL) {
		return;
	}

	if (args->this != NULL) free(args->this);

	free_image_extract(args->info);

	if (args->dim_start_end != NULL) {
		int i=0;
		while (i<args->volume->dim_nb) {
			if (args->dim_start_end[i] != NULL) free(args->dim_start_end[i]);
			i++;
		}
		free(args->dim_start_end);
	}

	if (args->volume != NULL) {
		if (args->volume->size != NULL) free(args->volume->size);
		if (args->volume->path != NULL) free(args->volume->path);

		if (args->volume->dim_name != NULL) {
			int i=0;
			while (i<args->volume->dim_nb) {
				if (args->volume->dim_name[i] != NULL) free(args->volume->dim_name[i]);
				i++;
			}
			free(args->volume->dim_name);
		}
		if (args->volume->starts != NULL) free(args->volume->starts);
		if (args->volume->steps != NULL) free(args->volume->steps);
		if (args->volume->dim_name_char != NULL) free(args->volume->dim_name_char);
		if (args->volume->dim_offset != NULL) free(args->volume->dim_offset);
		if (args->volume->slice_size != NULL) free(args->volume->slice_size);

		free(args->volume);
	}

	// set shared pointer to requests to null
	args->general_info = NULL;

	free(args);
}
