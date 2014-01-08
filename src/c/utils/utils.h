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
#include <math.h>

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
void		write_http_error(FILE * socket, char * text, char * status);

void free_null_terminated_char_2D_array(char ** strings);
inline char * appendCharacterToTempTokenBuffer(char * tempTokenBuffer,
		int * tempTokenBufferSize,
		int * tempTokenBufferCapacity,
					      char character);
char* strupper( char* s );
char* strlower( char* s );
char		*array_2D_to_array_1D(char **str);

// takes an unsigned value and maps it from its original range to a new range.
// Note that the range arguments denote the bit length, e.g 8,16,etc. up to a 64 maximum!!
inline unsigned long long mapUnsignedValue(unsigned char fromBitRange, unsigned char toBitRange, unsigned long long value);

#endif /* !__UTILS_H__ */













