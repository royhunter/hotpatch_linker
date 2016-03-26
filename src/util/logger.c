#include <util.h>



#include <obj.h>




int debug = 0;

void debug_print(const char *fmt,...)
{
	va_list args;

	if (!debug) {
		return;
    } else {
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
    }
}


