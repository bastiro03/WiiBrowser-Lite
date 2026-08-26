/*
 * newlibcompat.c
 *
 * The prebuilt libcurl.a was compiled against an older newlib which
 * exported the __ctype_ptr__ symbol (a pointer to the ctype bits table).
 * The current devkitPPC newlib renamed the table to _ctype_ and no longer
 * defines __ctype_ptr__.  Provide it so libcurl links cleanly.
 */
#include <stddef.h>

extern const char _ctype_[];
const char *__ctype_ptr__ = _ctype_;