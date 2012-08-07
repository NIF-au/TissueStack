#include "tile_requests.h"

void key_destroyed(gpointer key) {
 if (key != NULL) free(key);
}

inline void empty_set_if_above_threshold(t_tile_requests * this) {
	if (this == NULL || this->set == NULL || g_hash_table_size(this->set) < MAX_REQUESTS) {
		return;
	}

	this->destroy(this);
	init_tile_requests_set(this);
}

inline int convertRequestIdAndTimeIntoNumericSetKey(char * id, char * timestamp) {
	// the key is the delta of the id - timestamp, both are numeric and need to be converted
	long long id_num = atoll(id);
	long long time_num = atoll(timestamp);

	return id_num - time_num;
}

void init_tile_requests_set(t_tile_requests * this) {
	this->set = g_hash_table_new_full(g_int_hash, g_int_equal, key_destroyed, NULL);

	this->add = add_tile_request;
	this->contains = contains_tile_request;
	this->destroy = destroy_tile_requests_set;
}

void add_tile_request(t_tile_requests * this, char * id, char * timestamp) {
	empty_set_if_above_threshold(this);

	int key = convertRequestIdAndTimeIntoNumericSetKey(id, timestamp);
	g_hash_table_replace(this->set, (int *) &key, (int *) &key);
}

short contains_tile_request(t_tile_requests * this, char * id, char * timestamp) {
	int key = convertRequestIdAndTimeIntoNumericSetKey(id, timestamp);

	return (g_hash_table_lookup_extended(this->set, (int *) &key, NULL, NULL) == TRUE) ? 1 : 0;
}

void destroy_tile_requests_set(t_tile_requests * this) {
	g_hash_table_destroy(this->set);
}
