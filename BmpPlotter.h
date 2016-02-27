#ifndef PLOTTER_CDROM_BMPPLOTTER_H
#define PLOTTER_CDROM_BMPPLOTTER_H

#include "common.h"

extern const long BMP_PLOTTER_STEPS_PER_PIXEL_X;
extern const long BMP_PLOTTER_STEPS_PER_PIXEL_Y;

void BmpPlotter_plot(
	Bitmap*      bmp,
	Controller*  ctr,
	Vars*        vars,
	Ui*          ui
);

#endif
