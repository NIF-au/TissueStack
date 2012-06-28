#include "utils.h"

inline short appendCharacterToTempTokenBuffer(
		char ** tempTokenBuffer,
		int * tempTokenBufferSize,
		int * tempTokenBufferCapacity,
		char character) {
	if (tempTokenBuffer == NULL) {
		return 0;
	}

	// add space for 25 more characters to the buffer
	if ((*tempTokenBufferSize) == *tempTokenBufferCapacity) {
		char *tmp = (char *) realloc(*tempTokenBuffer, ((*tempTokenBufferCapacity) + 25) * sizeof(char *));

		if (tmp == NULL) {
			free(tempTokenBuffer);
			return 0;
		}
		*tempTokenBuffer = tmp;
		(*tempTokenBufferCapacity) += 25;
	}

	// add the character
	(*tempTokenBuffer)[*tempTokenBufferSize] = character;
	// increment the buffer size
	(*tempTokenBufferSize)++;

	return 1;
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
		  if (!appendCharacterToTempTokenBuffer(&tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, delimiter)) break;
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
			  if (!appendCharacterToTempTokenBuffer(&tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, '\0')) break;
			  dest[j] = strdup(tempTokenBuffer);
			  j++;
		  }
		  // reset token length and temp token buffer size
		  tokenLength = 0;
		  tempTokenBufferSize = 0;
	  } else {
		  // add character
		  if (!appendCharacterToTempTokenBuffer(&tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, charAtI)) break;
		  // increment token length counter
		  tokenLength++;
	  }

      i++;
    }


  // potential token leftover before end => copy over as well
  if (tokenLength > 0) {
	  // add '\0' to temp buffer to indicate end
	  if (!appendCharacterToTempTokenBuffer(&tempTokenBuffer, &tempTokenBufferSize, &tempTokenBufferCapacity, '\0')) return NULL;
	  // copy temp token buffer contents over into destination
	  dest[j] = strdup(tempTokenBuffer);
  }

  // terminate 2D array with NULL
   dest[++j] = NULL;

   // free temp buffer
   if (tempTokenBuffer != NULL) free(tempTokenBuffer);

  return (dest);
}

int		main(int argc, char ** args)
{
  if (argc < 2) {
	  printf("Please hand in the string!!!!\n");
	  exit(1);
  }
  printf("Handed in string: *%s*\n", args[1]);
  char		**results = tokenizeString(args[1], '|', '\\');

  if (results == NULL) {
	  printf("Empty\n");
	  exit(1);
  }

  // loop over results
  char * result = NULL;
  int i = 0;
  while ((result = results[i]) != NULL) {
	  printf("Token %i => *%s*\n", i+1, result);
	  if (result != NULL) free(result);
	  i++;
  }

  if (results != NULL) free(results);

  return 1;
}
