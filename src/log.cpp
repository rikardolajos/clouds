#include "log.h"

#include "GL/glew.h"

#include <stdio.h>
#include <stdarg.h>
#include <atltime.h>

void log_init()
{
	if (ERROR_LOG == 1) {
		FILE* file = fopen("../log.txt", "a");
		if (!file) fprintf(stdout, "Could not open log.txt for appending!");
		CString t = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M");
		fprintf(file, "\n\n[%s]\n", t);
		fclose(file);
	}
}

void log(const char* message, ...)
{
	va_list args;

	va_start(args, message);

	if (ERROR_LOG == 1) {
		FILE* file = fopen("../log.txt", "a");
		if (!file) fprintf(stdout, "Could not open log.txt for appending!");
		vfprintf(file, message, args);
		fclose(file);
	}
	else {
		vfprintf(stderr, message, args);
	}

	va_end(args);
}

void log_opengl_error()
{
	int e = glGetError();
	if (e != 0) {
		printf("OpenGL Error: 0x%x\n\n", e);
	}
}

void log_opengl_clear_errors()
{
	while (glGetError() != 0);
}