#ifndef __UTILS_H__
#define __UTILS_H__

#include <core.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

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
void free_t_string_buffer(t_string_buffer * buffer);

/** creates a directory (recursively incl. sub directories) */
t_string_buffer * createDirectory(char * path, mode_t mode);

void		write_http_header(FILE * socket, char * status, char * image_type);

void free_null_terminated_char_2D_array(char ** strings);
inline short appendCharacterToTempTokenBuffer(char ** tempTokenBuffer,
		int * tempTokenBufferSize,
		int * tempTokenBufferCapacity,
					      char character);
char* strupper( char* s );
char* strlower( char* s );

#endif /* !__UTILS_H__ */













