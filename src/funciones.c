#include <pebble.h>
#include "funciones.h"

char* subString (const char* input, int offset, int len, char* dest)
{
  int input_len = strlen (input);
  if (offset + len > input_len)
  {
     return NULL;
  }
  strncpy (dest, input + offset, len);
  return dest;
}