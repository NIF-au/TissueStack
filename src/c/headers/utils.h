#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** for string tokenization **/
int countTokens(char *buffer, char delimiter, char escape);
char ** tokenizeString(char *buffer, char delimiter, char escape);

#endif /* !__UTILS_H__ */













