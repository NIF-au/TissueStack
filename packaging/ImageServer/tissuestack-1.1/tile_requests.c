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
#include "tile_requests.h"

void key_destroyed(gpointer key) {
	if (key != NULL) free(key);
}

void value_destroyed(gpointer value) {
	if (value != NULL) free(value);
}

void empty_hash_if_above_threshold(t_tile_requests * this) {
	if (this == NULL || this->hash == NULL || g_hash_table_size(this->hash) < MAX_REQUESTS) {
		return;
	}

	this->destroy(this);
	init_tile_requests(this);
}

inline int convertRequestIdAndTimeIntoNumericDifference(char * id, char * timestamp) {
	// the id is longer by up to 2 digits so we right pad the timstamp with 0s to be of the same length and produce the
	// difference which makes for a smaller number that can than be compared to the previously stored difference
	// to determine whether a request has been superseded by a newer request

	int id_len = strlen(id);
	int time_len = strlen(timestamp);

	char * zeroPad = malloc(sizeof(*zeroPad) * (id_len-time_len+1));
	int i=0;
	while(i<id_len-time_len) {
		zeroPad[i] = '9';
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
	this->locked = 0;
}

void add_tile_request(t_tile_requests * this, char * id, char * timestamp) {
  if (id == NULL || timestamp == NULL || this->locked)
    return;

	gint64 * id_num = malloc(sizeof(*id_num));
	*id_num = atoll(id);
	int * requestDelta = malloc(sizeof(*requestDelta));
	*requestDelta = convertRequestIdAndTimeIntoNumericDifference(id, timestamp);
	gint64 * storedDelta = g_hash_table_lookup(this->hash, id_num);

	if (storedDelta != NULL && ((int)*storedDelta == *requestDelta)) {
		free(id_num);
		id_num = NULL;
		return;
	}

  	this->locked = 1;
  	empty_hash_if_above_threshold(this);
	g_hash_table_replace(this->hash, id_num, requestDelta);
	this->locked = 0;
}

short is_expired_tile_request(t_tile_requests * this, char * id, char * timestamp) {
	// see whether we have a difference value for a given id already
  if (this == NULL || id == NULL || timestamp == NULL)
    return 0;

    gint64 * storedValue = NULL;
	gint64 * id_num = malloc(sizeof(*id_num));
	*id_num = atoll(id);

	if ((storedValue = g_hash_table_lookup(this->hash, id_num)) == NULL) {
		this->add(this, id, timestamp); // first time add
		return 0;
	}

	int requestDelta = convertRequestIdAndTimeIntoNumericDifference(id, timestamp);
	if (requestDelta < (int)*storedValue) {
		return 1;
	}

	return 0;
}

void destroy_tile_requests(t_tile_requests * this) {
	if (this == NULL || this->hash == NULL) return;
	g_hash_table_destroy(this->hash);
}
