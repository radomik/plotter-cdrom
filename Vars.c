#include "Vars.h"

void Vars_new(Vars* this) {
	this->fullFileName[0]   = '\0';
	this->scale             = 1.0;
	this->plotFile          = NULL;
	this->coordinateCount   = 0;
	Vars_initXyMinMax(this);
}

void Vars_initXyMinMax(Vars* this) {
	this->xMin = this->yMin =  1000000;
	this->xMax = this->yMax = -1000000;
}

int Vars_openPlotFile(Vars* this, const char* name, const char* path, char* errBuf, size_t errBufSize) {
	if (! (this->plotFile=fopen(path, "rb"))) {
		snprintf(errBuf, errBufSize, "Failed to open file '%s' [%s]", name, geterr());
		l_error("%s", errBuf);
		return -1;
	}
	return 0;
}

void Vars_closePlotFile(Vars* this) {
	if (this->plotFile) {
		fclose(this->plotFile);
		this->plotFile = NULL;
	}
}

b8 Vars_isPlotFileOpened(const Vars* this) {
	return this->plotFile != NULL;
}
