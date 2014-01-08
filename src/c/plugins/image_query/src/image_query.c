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

#include "image_extract.h"

void		*init(void *args)
{
	  t_image_extract	*image_args;
	  t_args_plug	*a;
	  a = (t_args_plug *)args;
	  prctl(PR_SET_NAME, "Image_plug");

	  LOG_INIT(a);

	  image_args = malloc(sizeof(*image_args));
	  image_args->total_slices_to_do = 0;
	  image_args->slices_done = 0;
	  image_args->step = 0;
	  image_args->dim_start_end = NULL;
	  image_args->root_path = NULL;
	  image_args->service = NULL;
	  image_args->image_type = NULL;
	  image_args->request_id = NULL;
	  image_args->request_time = NULL;
	  pthread_mutex_init(&image_args->mut, NULL);
	  pthread_mutex_init(&image_args->percent_mut, NULL);
	  pthread_cond_init(&image_args->cond, NULL);
	  InitializeMagick("./");
	  a->this->stock = (void*)image_args;

	  INFO("Image Extract Plugin: Started");

	  // free command line args
	  a->destroy(a);

	  return (NULL);
}

void			*start(void *args) {
	INFO("Image Query Plugin: Entered");

	t_image_extract	*image_args;
	//t_image_extract	*image_args_tmp;
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

	//image_args_tmp = (t_image_extract*)a->this->stock;
	image_args = create_image_struct();
	image_args->dim_nb = volume->dim_nb;

	write_http_header(socketDescriptor, "200 OK", "png");
	fclose(socketDescriptor);

	return NULL;
}

t_image_extract	*create_image_struct()
{
  t_image_extract	*image_args;

  image_args = malloc(sizeof(*image_args));
  image_args->total_slices_to_do = 0;
  image_args->slices_done = 0;
  image_args->step = 0;
  image_args->dim_start_end = NULL;
  image_args->root_path = NULL;
  image_args->service = NULL;
  image_args->image_type = NULL;
  image_args->request_id = NULL;
  image_args->request_time = NULL;
  pthread_mutex_init(&image_args->mut, NULL);
  pthread_cond_init(&image_args->cond, NULL);
  return (image_args);
}

void		*unload(void *args) {
	t_image_extract	*image_args = NULL;
	t_args_plug	*a = NULL;

	a = (t_args_plug *)args;
	image_args = (t_image_extract *)a->this->stock;
	free_image_extract(image_args);

	DestroyMagick();

	INFO("Image Query Plugin: Unloaded");

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
