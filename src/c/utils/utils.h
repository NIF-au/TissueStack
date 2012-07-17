#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

/** for string tokenization **/
int countTokens(char *buffer, char delimiter, char escape);
char ** tokenizeString(char *buffer, char delimiter, char escape);
/** for string appending **/
typedef struct		s_string_buffer t_string_buffer	;
struct s_string_buffer {
	char * buffer;
	int size;
	int capacity;
};

t_string_buffer * appendToBuffer(t_string_buffer * buffer, char * someString);

/** creates a directory (recursively incl. sub directories) */
t_string_buffer * createDirectory(char * path, mode_t mode);

#endif /* !__UTILS_H__ */













