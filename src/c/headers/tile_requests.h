#ifndef	__TILE_REQUESTS__
#define __TILE_REQUESTS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtk/gtk.h"

#define MAX_REQUESTS 10000000

typedef struct		s_tile_requests	t_tile_requests;

struct			s_tile_requests
{
	GHashTable * set;
	void 		(*add) (t_tile_requests * this, char * id, char * timestamp);
	short		(*contains) (t_tile_requests * this, char * id, char * timestamp);
	void 		(*destroy) (t_tile_requests * this);
};

void init_tile_requests_set(t_tile_requests * this);
void add_tile_request(t_tile_requests * this, char * id, char * timestamp);
short contains_tile_request(t_tile_requests * this, char * id, char * timestamp);
void destroy_tile_requests_set(t_tile_requests * this);

#endif	/* __TILE_REQUESTS__ */
