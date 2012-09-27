#include "memory_mapping.h"

void mm_key_destroyed(gpointer key) {
	if (key != NULL) free(key);
}

void mm_value_destroyed(gpointer value) {
	// clean up after ourselves
	if (value != NULL) {
		t_memory_mapped_file * file = (t_memory_mapped_file *) value;
		if (file->data != NULL) munmap(file->data, file->file_info->st_size);
		if (file->file_info != NULL) free(file->file_info);
		free(file);
	}
}

void init_memory_mapping(t_memory_mapping * this) {
	this->hash = g_hash_table_new_full(g_str_hash, g_str_equal, mm_key_destroyed, mm_value_destroyed);

	this->add = add_memory_mapped_file;
	this->get = get_memory_mapped_data;
}

void add_memory_mapped_file(t_memory_mapping * this, char * path) {
	if (this == NULL || path == NULL || this->get(this, path) != NULL) return;

	// make a copy of path
	char * copyOfPath = strdup(path);

	// hash entry exists already
	if (this->get(this, copyOfPath) != NULL) return;

	// memory map file
	t_memory_mapped_file * mm_file = malloc(sizeof(* mm_file));

	int fd = open (copyOfPath, O_RDONLY);
	if (fd == -1) {
		//DEBUG("Failed to open file for memory mapping\n");
		printf("Failed to open file for memory mapping\n");
		return;
	}

	mm_file->file_info = malloc(sizeof(* mm_file->file_info));
	if (fstat (fd, mm_file->file_info) == -1) {
		//DEBUG("Failed to obtain file stats for memory mapping\n");
		printf("Failed to obtain file stats for memory mapping\n");
		return;
	}

	mm_file->data = mmap (0, mm_file->file_info->st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (mm_file == MAP_FAILED) {
		//DEBUG("Failed to memory map given file\n");
		printf("Failed to memory map given file\n");
		return;
	}

	g_hash_table_replace(this->hash, (gpointer *) copyOfPath, (gpointer *) mm_file);
}

char * get_memory_mapped_data(t_memory_mapping * this, char * path) {
	t_memory_mapped_file * mm_file = g_hash_table_lookup(this->hash, (gpointer *) path);
	if (mm_file == NULL || mm_file->data == NULL) return NULL;

	return mm_file->data;
}

void destroy_memory_mapping(t_memory_mapping * this) {
	if (this == NULL || this->hash == NULL) return;
	g_hash_table_destroy(this->hash);
}
