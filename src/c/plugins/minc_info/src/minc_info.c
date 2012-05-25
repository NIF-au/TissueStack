#include "minc_info.h"

volume_input_struct *volume = NULL;

void *init(void *args) {
	if (args == NULL) {
		return NULL;
	}

	volume = malloc(sizeof(volume_input_struct));

	// open the minc file and read information
	Volume in_volume = malloc(sizeof(Volume));
	char * axis_order[3] = { MIzspace, MIyspace, MIxspace };

	if (start_volume_input(args, MAX_VAR_DIMS, axis_order, NC_UNSPECIFIED, TRUE, 0.0, 0.0, TRUE,
			&in_volume, (minc_input_options *) NULL, volume) != OK) {
		return NULL;
	}

	delete_volume(in_volume);

	return NULL;
}

void *start(void *args) {
	return volume;
}

void *unload(void *args) {
	// clean up after ourselves
	delete_volume_input(volume);

	return NULL;
}
