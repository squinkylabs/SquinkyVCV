
#include "SqLog.h"

#include <cstdarg>
#include <cstdio>

namespace SqLog {
#ifdef __PLUGIN

#include "logger.hpp"

void log(Level level, const char* filename, int line, const char* format, ...) {
	rack::logger::log(level, filename, line, format, ...);
}

#else

static const int levelColors[] = {
35,
34,
33,
31
};

static const char* const levelLabels[] = {
	"debug",
	"info",
	"warn",
	"fatal"
};

static void logVa(Level level, const char* filename, int line, const char* format, va_list args) {
	auto outputFile = stderr;
    fprintf(outputFile, "\x1B[%dm", levelColors[level]);
	fprintf(outputFile, "[%s %s:%d] ", levelLabels[level], filename, line);
	fprintf(outputFile, "\x1B[0m");
	vfprintf(outputFile, format, args);
	fprintf(outputFile, "\n");
	fflush(outputFile);

}

void log(Level level, const char* filename, int line, const char* format, ...) {
	va_list args;
	va_start(args, format);
	logVa(level, filename, line, format, args);
	va_end(args);
}

#endif

}