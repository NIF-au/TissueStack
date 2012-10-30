#include "utils.h"

inline char * appendCharacterToTempTokenBuffer(
		char * tempTokenBuffer,
		int * tempTokenBufferSize,
		int * tempTokenBufferCapacity,
		char character) {
	if (tempTokenBuffer == NULL) {
		return NULL;
	}

	// add space for 25 more characters to the buffer
	if ((*tempTokenBufferSize) == *tempTokenBufferCapacity) {
		char *tmp = (char *) realloc(tempTokenBuffer, ((*tempTokenBufferCapacity) + 25) * sizeof(char *));

		if (tmp == NULL) {
			free(tempTokenBuffer);
			return NULL;
		}
		tempTokenBuffer = tmp;
		(*tempTokenBufferCapacity) += 25;
	}

	// add the character
	(tempTokenBuffer)[*tempTokenBufferSize] = character;
	// increment the buffer size
	(*tempTokenBufferSize)++;

	return tempTokenBuffer;
}

int countTokens(char *buffer, char delimiter, char escape)
{
  // preliminary checks
  if (buffer == NULL) {
	  return 0;
  }

  int		i = 0;
  int		count = 0;

  int tokenLength = 0;
  char charAtI = '\0';

  while ((charAtI = buffer[i]) != '\0')
  {
	  // if we have an escaped delimiter => skip over it
	  if (charAtI == escape && buffer[i+1] != '\0' && buffer[i+1] == delimiter) {
		  i += 2;
		  tokenLength += 2;

		  // fetch present buffer position content
		  charAtI = buffer[i];
	  }

	  // we encountered the delimiter
	  if (charAtI == delimiter) {
		  // increment count only if we have at least 1 character of a token
		  if (tokenLength>0) {
			  count++;
		  }
		  // reset token length
		  tokenLength = 0;
	  } else {
		  tokenLength++;
	  }

      i++;
    }

  // potential token leftover before end
  if (tokenLength > 0) {
	  count++;
  }

  return (count);
}

char ** tokenizeString(char *buffer, char delimiter, char escape)
{
  // preliminary checks
  if (buffer == NULL) {
	  return NULL;
  }

  int	i = 0;
  int	j = 0;
  char	**dest = NULL;

  int 	tokenCount = countTokens(buffer, delimiter, escape);

  // allocate result array
  dest = (char **) malloc((tokenCount + 1) * sizeof(*dest));

  // a reusable token cutter buffer that gets increased if needed
  int tempTokenBufferSize = 0;
  int tempTokenBufferCapacity = 50;
  char * tempTokenBuffer = (char *) malloc(tempTokenBufferCapacity * sizeof(tempTokenBuffer));

  int tokenLength = 0;
  char charAtI = '\0';

  while ((charAtI = buffer[i]) != '\0')
  {
	  // if we have an escaped delimiter => skip over it
	  if (charAtI == escape && buffer[i+1] == delimiter) {
		  // add delimiter
		  if ((tempTokenBuffer = appendCharacterToTempTokenBuffer(tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, delimiter)) == NULL) break;
		  i += 2;
		  ++tokenLength;

		  // fetch present buffer position content
		  charAtI = buffer[i];
	  }

	  // we encountered the delimiter => end of token
	  if (charAtI == delimiter) {
		  // copy temp token buffer contents over into destination
		  if (tokenLength>0) {
			  // add '\0' to temp buffer to indicate end
			  if ((tempTokenBuffer = appendCharacterToTempTokenBuffer(tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, '\0')) == NULL) break;
			  dest[j] = strdup(tempTokenBuffer);
			  j++;
		  }
		  // reset token length and temp token buffer size
		  tokenLength = 0;
		  tempTokenBufferSize = 0;
	  } else {
		  // add character
		  if ((tempTokenBuffer = appendCharacterToTempTokenBuffer(tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, charAtI)) == NULL) break;
		  // increment token length counter
		  tokenLength++;
	  }

      i++;
    }


  // potential token leftover before end => copy over as well
  if (tokenLength > 0) {
	  // add '\0' to temp buffer to indicate end
	  if ((tempTokenBuffer = appendCharacterToTempTokenBuffer(tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, '\0')) == NULL) return NULL;
	  // copy temp token buffer contents over into destination
	  dest[j] = strdup(tempTokenBuffer);
	  ++j;
  }

  // terminate 2D array with NULL
   dest[j] = NULL;

   // free temp buffer
   if (tempTokenBuffer != NULL) free(tempTokenBuffer);

  return (dest);
}

t_string_buffer *  appendToBuffer(t_string_buffer * buffer, char * someString) {
	if (someString == NULL) {
		return NULL;
	}

	// create new buffer if null
	if (buffer == NULL) {
		buffer = (t_string_buffer *) malloc(sizeof(*buffer));
		buffer->size = 0;
		buffer->capacity = 100;
		buffer->buffer = (char *) malloc(sizeof(buffer->buffer) * buffer->capacity);
	}

	// now let's look at the string we are meant to append
	int lengthOfSomeString = strlen(someString);
	int potentialBufferExcess = buffer->capacity - buffer->size - lengthOfSomeString;

	// do we exceed the buffer size ?
	if (potentialBufferExcess < 0) {
		// get some more buffer capacity => old size + lengthOfSomeString + 100 chars)
		char *tmp = (char *) realloc(buffer->buffer, (buffer->size + lengthOfSomeString + 100) * sizeof(tmp));

		if (tmp == NULL) {
			free(buffer->buffer);
			return NULL;
		}
		buffer->buffer = tmp;
		buffer->capacity = buffer->size + lengthOfSomeString + 100;
	}

	// copy string into buffer
	int i=0;
	for (i=0;i<lengthOfSomeString;i++) {
		buffer->buffer[buffer->size] = someString[i];
		buffer->size++;
	}
	// terminal '\0'
	buffer->buffer[buffer->size] = '\0';

	return buffer;
}
void free_t_string_buffer(t_string_buffer * buffer) {
	if (buffer == NULL) return;

	if (buffer->buffer != NULL) free(buffer->buffer);
	free(buffer);
}

// Returns path for success, omitting last "/" OR NULL in case of failure
t_string_buffer * createDirectory(char * path, mode_t mode) {
	if (path == NULL || strlen(path) == 0) {
		return NULL;
	}

	// first tokenize path
	const int numberOfDirectories = countTokens(path, '/', '\\');
	if (numberOfDirectories == 0) {
		return NULL;
	}
	char	**directories = tokenizeString(path, '/', '\\');
	if (directories == NULL) {
		return NULL;
	}

	int i=0;
	t_string_buffer * pathWithoutSlashAtEnd = NULL;

	// loop through list and create them all
	while (directories[i] != NULL) {
		char * dir = directories[i];
		pathWithoutSlashAtEnd = appendToBuffer(pathWithoutSlashAtEnd, "/");
		pathWithoutSlashAtEnd = appendToBuffer(pathWithoutSlashAtEnd, dir);

		if (mkdir(pathWithoutSlashAtEnd->buffer, mode) < 0 && errno != EEXIST) {
			// if we we didn't fail because we exist already => abort
			ERROR("Failed to create directory: %s\n", strerror(errno));
			if (pathWithoutSlashAtEnd != NULL) { // free
				if (pathWithoutSlashAtEnd->buffer != NULL) {
					free(pathWithoutSlashAtEnd->buffer);
				}
				free(pathWithoutSlashAtEnd);
			}
			pathWithoutSlashAtEnd = NULL;
		}

		free(dir);
		i++;
	}

	free(directories);

	return pathWithoutSlashAtEnd;
}

char* strupper( char* s )
{
  if (s == NULL) return NULL;

  int i = 0;
  while (s[i] != '\0') {
	  s[i] = toupper(s[i]);
    i++;
  }
  return s;
}

char* strlower( char* s )
{
  if (s == NULL) return NULL;

  int i = 0;
  while (s[i] != '\0') {
    s[i] = tolower(s[i]);
    i++;
  }
  return s;
}


void		write_http_header(FILE * socket, char * status, char * image_type)
{
	t_string_buffer * header = appendToBuffer(NULL, "HTTP/1.1 ");
	header = appendToBuffer(header, status); // HTTP STATUS
	header = appendToBuffer(header, "\r\nDate: Thu, 20 May 2004 21:12:11 GMT\r\n"); // Date (in the past)
	header = appendToBuffer(header, "Connection: close\r\n"); // Connection header (close)
	header = appendToBuffer(header, "Server: Tissue Stack Image Server\r\n"); // Server header
	header = appendToBuffer(header, "Accept-Ranges: bytes\r\n"); // Accept-Ranges header
	/*
	char contLen[150];
	sprintf(contLen, "Content-Length: %lu\r\n", content_length);
	//header = appendToBuffer(header, contLen); // Content-Length header
	 */
	if (image_type != NULL)
	{
		header = appendToBuffer(header, "Content-Type: image/"); // Content-Type header
		header = appendToBuffer(header, image_type); // image type
	} else {
		header = appendToBuffer(header, "Content-Type: text/html");
	}
	header = appendToBuffer(header, "\r\nAccess-Control-Allow-Origin: *\r\n"); // allow cross origin requests
	header = appendToBuffer(header, "Last-Modified: Thu, 20 May 2004 21:12:11 GMT\r\n\r\n"); // last modified header in the past

	write(fileno(socket), header->buffer, header->size);

	free(header->buffer);
	free(header);
}

short testBufferAppend() {
	printf("\t*) String Append => ");

    t_string_buffer * a_buffer = (t_string_buffer *) malloc(sizeof(*a_buffer));
	a_buffer->size = 0;
	a_buffer->capacity = 20;
	a_buffer->buffer = (char *) malloc(sizeof(a_buffer->buffer) * a_buffer->capacity);

    a_buffer = appendToBuffer(a_buffer, "hello");
	a_buffer = appendToBuffer(a_buffer, " world ");
	a_buffer = appendToBuffer(a_buffer, " : here I come again and again ");

	if (a_buffer->size != 43 || a_buffer->capacity != 143 || strlen(a_buffer->buffer) != 43) {
		printf("FAILED !\n");
	} else {
		printf("PASSED.\n");
	}
    if (a_buffer->buffer != NULL) free(a_buffer->buffer);
    free(a_buffer);

	return 1;
}

short testTokenizer() {
	  printf("\t*) Tokenizer => ");

	  char * test_string ="\\\\|| | x|\\|kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX 484375435 || > < | < O >";
	  char	**results = tokenizeString(test_string, '|', '\\');

	  if (results == NULL) {
		  printf("FAILED !\n");
		  return 0;
	  }

	  // loop over results
	  char * result = NULL;
	  int i = 0;
	  while ((result = results[i]) != NULL) {
		  if (strlen(result) == 0) {
			  printf("FAILED !\n");
			  return 0;
		  }
		  if (result != NULL) free(result);
		  i++;
	  }
	  if (results != NULL) free(results);

	  printf("PASSED.\n");

	  return 1;
}

short testDirectoryCreation() {
	  printf("\t*) Directory Creation => ");

	  t_string_buffer * result = NULL;

	  result = createDirectory(NULL, 0775);
	  if (result != NULL) {
		  free(result->buffer);
		  free(result);
		  printf("FAILED !\n");
		  return 0;
	  }

	  result = createDirectory("/boot", 0775);
	  if (result == NULL) {
		  printf("FAILED !\n");
		  return 0;
	  }
	  free(result->buffer);
	  free(result);

	  struct timeval  tv;
	  gettimeofday(&tv, NULL);
	  char buffer[75];
	  sprintf(buffer, "/tmp/%ld", tv.tv_sec);
	  result = createDirectory(buffer, 0775);
	  if (result == NULL) {
		  printf("*%s*%s*\n", result->buffer, buffer);
		  printf("FAILED !\n");
		  return 0;
	  } else if (strcmp(buffer, result->buffer) == 0) {
		  free(result->buffer);
		  free(result);
		  printf("PASSED.\n");
	  } else {
		  printf("FAILED !\n");
		  printf("*%s*%s*\n", result->buffer, buffer);
		  free(result->buffer);
		  free(result);
		  return 0;
	  }

	  return 1;
}

short testToUpAndLow() {
	  printf("\t*) To Upper And Lower => ");

	  char * t = strdup("test1");

	  if (strcmp(strupper(t), "TEST1") != 0) {
		  printf("FAILED !\n");
	  }

	  free(t);

	  t = strdup("TEST2");
	  if (strcmp(strlower(t), "test2") != 0) {
		  printf("FAILED !\n");
	  }

	  free(t);

	  printf("PASSED.\n");

	  return 1;
}

void free_null_terminated_char_2D_array(char ** strings) {
	if (strings == NULL) return;

	int i=0;
	while (strings[i] != NULL) {
		free(strings[i]);
		i++;
	}
	free(strings);
}

char		*array_2D_to_array_1D(char **src)
{
  int		i = 0;
  int		len = 0;
  char		*str;
  int		j;
  int		k = 0;

  while (src[i] != NULL)
    {
      len += strlen(src[i]);
      i++;
    }
  str = malloc((len + i + 1) * sizeof(*str));
  i = 0;
  while (src[i] != NULL)
    {
      j = 0;
      while (src[i][j] != '\0')
	{
	  str[k] = src[i][j];
	  k++;
	  j++;
	}
      str[k++] = ' ';
      i++;
    }
  str[k] = '\0';
  return (str);
}

/** TESTS **/
/*
int		main(int argc, char ** args)
{
	printf("Running tests ....\n\n");
   // TOKENIZER TEST
   if (!testTokenizer()) {
	   printf("Tests aborted because of errors!\n");
	   exit(0);
   }
   // BUFFER APPEND TEST
   if (!testBufferAppend()) {
	   printf("Tests aborted because of errors!\n");
	   exit(0);
   }
   // DIRECTORY CREATION TEST
   if (!testDirectoryCreation()) {
	   printf("Tests aborted because of errors!\n");
	   exit(0);
   }

   // UPPER AND LOWER TEST
   if (!testToUpAndLow()) {
	   printf("Tests aborted because of errors!\n");
	   exit(0);
   }

	printf("\nAll tests finished without error.\n");

	return 1;
}
*/
