#ifndef INC_CONSOLE_H
#define INC_CONSOLE_H

#include <stdarg.h>

void console_init();
void cgetchar(int c);
void cprintf(const char *fmt, ...);
void panic(const char *fmt, ...);

#define assert(x)                                                   \
({                                                                  \
    if (!(x)) {                                                     \
        cprintf("%s:%d: assertion failed.\n", __FILE__, __LINE__);  \
        while(1) ;                                                  \
    }                                                               \
})

#endif