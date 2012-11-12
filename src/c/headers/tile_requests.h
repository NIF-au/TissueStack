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
#ifndef	__TILE_REQUESTS__
#define __TILE_REQUESTS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtk/gtk.h"

#define MAX_REQUESTS 10000

typedef struct		s_tile_requests	t_tile_requests;

struct			s_tile_requests
{
	GHashTable * hash;
	void 		(*add) (t_tile_requests * this, char * id, char * timestamp);
	short		(*is_expired) (t_tile_requests * this, char * id, char * timestamp);
	void 		(*destroy) (t_tile_requests * this);
};

void init_tile_requests(t_tile_requests * this);
void add_tile_request(t_tile_requests * this, char * id, char * timestamp);
short is_expired_tile_request(t_tile_requests * this, char * id, char * timestamp);
void destroy_tile_requests(t_tile_requests * this);

inline int convertRequestIdAndTimeIntoNumericDifference(char * id, char * timestamp);
inline void empty_hash_if_above_threshold(t_tile_requests * this);

#endif	/* __TILE_REQUESTS__ */
