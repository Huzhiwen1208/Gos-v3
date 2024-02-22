#include "mod.h"

void DebugHandle(const char *file, i32 line, const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    i32 i = Vsprintf(buf, fmt, args);
    va_end(args);
    PrintWithColor(CYAN,"DEBUG: ");
    PrintWithColor(CYAN,"[%s:%d]",file,line);
    PrintWithColor(CYAN,buf);

}

void TraceHandle(const char *file, i32 line, const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    i32 i = Vsprintf(buf, fmt, args);
    va_end(args);
    PrintWithColor(GRAY, "TRACE: ");
    PrintWithColor(GRAY,"[%s:%d]",file,line);
    PrintWithColor(GRAY,buf);
}

void InfoHandle(const char *file, i32 line, const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    i32 i = Vsprintf(buf, fmt, args);
    va_end(args);
    PrintWithColor(GREEN, "INFO: ");
    PrintWithColor(GREEN,"[%s:%d]",file,line);
    PrintWithColor(GREEN,buf);
}

void WarnHandle(const char *file, i32 line, const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    i32 i = Vsprintf(buf, fmt, args);
    va_end(args);
    PrintWithColor(YELLOW, "WARN: ");
    PrintWithColor(YELLOW,"[%s:%d]",file,line);
    PrintWithColor(YELLOW,buf);
}

void ErrorHandle(const char *file, i32 line, const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    i32 i = Vsprintf(buf, fmt, args);
    va_end(args);
    PrintWithColor(RED, "ERROR: ");
    PrintWithColor(RED,"[%s:%d]",file,line);
    PrintWithColor(RED,buf);
}