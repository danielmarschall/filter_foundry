/* this hack (to fool MPW linker) is due to
   http://preserve.mactech.com/articles/develop/issue_15/117-131_Q_As.html */

#include <stdio.h>
#include <mactypes.h>

// These calls won't actually be called by sprintf.
size_t fwrite (const void *a, size_t b, size_t c, FILE *d) {
	DebugStr("\pstubs-mpwstdio.c: fwrite called (THIS SHOULD NOT HAPPEN)");
	return 0;
}
int _flsbuf(unsigned char a, FILE *b) {
	DebugStr("\pstubs-mpwstdio.c: _flsbuf called (THIS SHOULD NOT HAPPEN)");
	return 0;
}       
