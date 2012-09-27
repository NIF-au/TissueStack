#include "tile_requests.h"

void key_destroyed(gpointer key) {
	if (key != NULL) free(key);
}

void value_destroyed(gpointer value) {
	if (value != NULL) free(value);
}

inline void empty_hash_if_above_threshold(t_tile_requests * this) {
	if (this == NULL || this->hash == NULL || g_hash_table_size(this->hash) < MAX_REQUESTS) {
		return;
	}

	this->destroy(this);
	init_tile_requests(this);
}

inline int convertRequestIdAndTimeIntoNumericDifference(char * id, char * timestamp) {
	// the id is longer by up to 3 digits so we right pad the timstamp with 0s to be of the same length and produce the
	// difference which makes for a smaller number that can than be compared to the previously stored difference
	// to determine whether a request has been superseded by a newer request

	int id_len = strlen(id);
	int time_len = strlen(timestamp);

	char * zeroPad = malloc(sizeof(*zeroPad) * (id_len-time_len+1));
	int i=0;
	while(i<id_len-time_len) {
		zeroPad[i] = '0';
		i++;
	}
	zeroPad[id_len-time_len] = '\0';

	char * zeroPaddedTimeStamp = malloc(sizeof(*zeroPaddedTimeStamp) * (id_len + 1));
	sprintf(zeroPaddedTimeStamp, "%s%s", timestamp, zeroPad);
	free(zeroPad);

	long long id_num = atoll(id);
	long long time_num = atoll(zeroPaddedTimeStamp);
	free(zeroPaddedTimeStamp);

	return time_num - id_num;
}

void init_tile_requests(t_tile_requests * this) {
	this->hash = g_hash_table_new_full(g_int64_hash, g_int64_equal, key_destroyed, value_destroyed);

	this->add = add_tile_request;
	this->is_expired = is_expired_tile_request;
	this->destroy = destroy_tile_requests;
}

void add_tile_request(t_tile_requests * this, char * id, char * timestamp) {
  empty_hash_if_above_threshold(this);

	int * requestDelta = malloc(sizeof(*requestDelta));
	*requestDelta = convertRequestIdAndTimeIntoNumericDifference(id, timestamp);

	gint64 * id_num = malloc(sizeof(*id_num));
	*id_num = atol(id);

	g_hash_table_replace(this->hash, id_num, requestDelta);
}

short is_expired_tile_request(t_tile_requests * this, char * id, char * timestamp) {
	// see whether we have a difference value for a given id already
	int * storedValue = NULL;

	gint64 * id_num = malloc(sizeof(*id_num));
	*id_num = atol(id);

	if ((storedValue = g_hash_table_lookup(this->hash, id_num)) == NULL) {
		return 0;
	}

	int requestDelta = convertRequestIdAndTimeIntoNumericDifference(id, timestamp);
	if (requestDelta < *storedValue) return 1;

	return 0;
}

void destroy_tile_requests(t_tile_requests * this) {
	if (this == NULL || this->hash == NULL) return;
	g_hash_table_destroy(this->hash);
}
