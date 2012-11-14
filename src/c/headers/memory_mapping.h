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
#ifndef	 __MEMORY_MAPPING__
#define __MEMORY_MAPPING__

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "gtk/gtk.h"

#define MAX_SIZE 1000

typedef struct		s_memory_mapped_file	t_memory_mapped_file;
struct			s_memory_mapped_file
{
	struct stat * file_info;
	char * data;
};

typedef struct		s_memory_mapping	t_memory_mapping;
struct			s_memory_mapping
{
	GHashTable * hash;
	void 		(*add) (t_memory_mapping * this, char * path);
	char * 		(*get) (t_memory_mapping * this, char * path);
};

void init_memory_mapping(t_memory_mapping * this);
void destroy_memory_mapping(t_memory_mapping * this);

void add_memory_mapped_file(t_memory_mapping * this, char * path);
char * get_memory_mapped_data(t_memory_mapping * this, char * path);

#endif	/*__MEMORY_MAPPING__ */
