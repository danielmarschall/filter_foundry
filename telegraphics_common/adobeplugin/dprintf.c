#include <stdarg.h>
#include <stdio.h>

#include "dbg.h"

int dprintf(const char *fmt, ...){
	char s[0x200];
	int n;
	va_list v;

	va_start(v,fmt);
	n = vsprintf(s,fmt,v);
	va_end(v);
	
	dbg(s);
	return n;
}
