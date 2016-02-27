#ifndef PLOTTER_CDROM_VARS_H
#define PLOTTER_CDROM_VARS_H

#include "common.h"

struct Vars {
	char     fullFileName[256];
	double   scale;
	FILE*    plotFile;
	int32_t  xMin, yMin;
	int32_t  xMax, yMax;
	uint32_t coordinateCount;
};

void Vars_new(Vars* this);

void Vars_initXyMinMax(Vars* this);

int Vars_openPlotFile(Vars* this, const char* name, const char* path, char* errBuf, size_t errBufSize);

void Vars_closePlotFile(Vars* this);

b8 Vars_isPlotFileOpened(const Vars* this);

#endif
