#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <ctype.h>

static inline
uint8_t ashex(const char *s)
{
	if(s[0] == '0' && tolower(s[1]) == 'x')
		s += 2;

	long v = strtol(s, NULL, 16);
	return (uint8_t) v;
}

#endif
