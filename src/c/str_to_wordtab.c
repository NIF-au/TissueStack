#include <stdlib.h>
#include <stdio.h>

char		*str_n_cpy(char *str, int position, int len, char e, char c)
{
  char		*dest;
  int		i;

  i = 0;
  dest = malloc((len + 1) * sizeof(*dest));
  while (i < len)
    {
      if (str[i + position] == e && str[i + position + 1] == c)
	position++;
      dest[i] = str[i + position];
      i++;
    }
  dest[i] = '\0';
  return (dest);
}

int		word_count(char *buff, char c, char e)
{
  int		i;
  int		count;

  i = 0;
  count = 0;
  if (buff[0] != c && buff[0] != '\0')
    count++;
  while (buff[i] != '\0')
    {
      if ((buff[i] == c && buff[i + 1] != c) && buff[i + 1] != '\0')
	{
	  if ((i - 1 >= 0) && buff[i - 1] != e)
	    count++;
	  else if ((i - 1) < 0)
	    count++;
	}
      i++;
    }
  return (count);
}

int		letter_count(char *buff, int position, char c, char e)
{
  int		i;

  i = 0;
  while (buff[position + i] != '\0')
    {
      if (buff[position + i] == c)
	{
	  if (position + i - 1 > 0)
	    {
	      if (buff[position + i - 1] != e)
		break;
	    }
	  else
	    break;
	}
      i++;
    }
  return (i);
}

char		**str_to_wordtab(char *buff, char c, char e)
{
  int		i;
  int		j;
  char		**dest;

  dest = malloc((word_count(buff, c, e) + 1) * sizeof(*dest));
  i = 0;
  j = 0;
  while (buff[i] != '\0')
    {
      if ((buff[i] != c || (i != 0 && buff[i - 1] == e && buff[i] == c)) && buff[i] != '\0')
	{
	  dest[j] = str_n_cpy(buff, i, letter_count(buff, i, c, e), e, c);
	  j++;
	  i += letter_count(buff, i, c, e);
	}
      if (buff[i] != '\0')
	i++;
    }
  dest[j] = NULL;
  return (dest);
}

int		main()
{
  char		yop[] = "Hello|worl\\|d";
  char		yop2[] = "\\H\\ello|worl\\|d\\";
  char		**result;
  int		i;

  result = str_to_wordtab(yop, '|', '\\');
  i = 0;
  while (result[i] != NULL)
    {
      printf("%s\n", result[i]);
      i++;
    }
  result = str_to_wordtab(yop2, '|', '\\');
  i = 0;
  while (result[i] != NULL)
    {
      printf("%s\n", result[i]);
      i++;
    }
}
