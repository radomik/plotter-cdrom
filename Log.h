#ifndef PLOTTER_CDROM_LOG_H
#define PLOTTER_CDROM_LOG_H

#include <stdio.h>
#include "types.h"

extern Log LOG_INSTANCE;

#define LOG_FILE "plotter-cdrom.log"

struct Log {
	FILE* logFile;
	char  e[128];
};

/// Get ERRNO text message
const char* geterr();

/// Open log file
/// @return 0 on success, -1 on error
int Log_open();

/// Close log file
void Log_close();

#define l_any(level, fmt, ...) if (LOG_INSTANCE.logFile) { fprintf(LOG_INSTANCE.logFile, level fmt "\n", __VA_ARGS__); fflush(LOG_INSTANCE.logFile); }
#define l_info(fmt, ...)  l_any("[INFO] ", fmt, __VA_ARGS__)
#define l_warn(fmt, ...)  l_any("[WARN] ", fmt, __VA_ARGS__)
#define l_error(fmt, ...) l_any("[ERRR] ", fmt, __VA_ARGS__)

#endif
