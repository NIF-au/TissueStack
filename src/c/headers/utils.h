#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** for string tokenization **/
int countTokens(char *buffer, char delimiter, char escape);
char ** tokenizeString(char *buffer, char delimiter, char escape);
/** for string appending **/
void appendToBuffer(char ** buffer,	int * bufferSize, int * bufferCapacity, char * someString);

#endif /* !__UTILS_H__ */













