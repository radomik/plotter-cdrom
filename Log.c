#include "Log.h"
#include <errno.h>
#include <string.h>

Log LOG_INSTANCE = {
	.logFile = NULL,
	.e = ""
};

const char* geterr() {
	snprintf(LOG_INSTANCE.e, sizeof(LOG_INSTANCE.e), "%s", strerror(errno));
	return LOG_INSTANCE.e;
}

int Log_open() {
	if (! (LOG_INSTANCE.logFile = fopen(LOG_FILE, "wb"))) {
		fprintf(stderr, "Error opening log file '%s' [%s]\n", LOG_FILE, geterr());
		return -1;
	}
	return 0;
}

void Log_close() {
	if (LOG_INSTANCE.logFile) {
		fclose(LOG_INSTANCE.logFile);
		LOG_INSTANCE.logFile = NULL;
	}
}
