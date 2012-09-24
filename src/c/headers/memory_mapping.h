#ifndef	 __MEMORY_MAPPING__
#define __MEMORY_MAPPING__

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

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
