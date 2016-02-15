#ifndef LOG_H
#define LOH_H

#define ERROR_LOG 0		/* 0 = print errors to stdout
						   1 = print errors to log.txt */

void log_init();
void log(const char* message, ...);
void log_opengl_error();
void log_opengl_clear_errors();

#endif