#pragma once

#pragma warning(push, 0)
#include <FL/Enumerations.H>
#pragma warning(pop)

#define OS_FONT FL_FREE_FONT
#define OS_FONT_SIZE 12

#ifdef _WIN32
#define OS_FONT_NAME "Segoe UI"
#define OS_FONT_NAME_ALT "Tahoma"
#elif defined(__linux__)
#define OS_FONT_NAME "Droid Sans"
#define OS_FONT_NAME_ALT "Sans"
#else
#define OS_FONT_NAME "Tahmoa"
#define OS_FONT_NAME_ALT "Arial"
#endif

void use_os_font(void);
