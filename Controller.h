#ifndef PLOTTER_CDROM_CONTROLLER_H
#define PLOTTER_CDROM_CONTROLLER_H

#include "common.h"

struct Controller {
	int     stepX;
	int     stepY;
	b8      currentPlotDown;
	uint8_t moveLength;
};

void Controller_new(Controller* this);

void Controller_toggleMoveLength(Controller* this);

void Controller_initOrExit(Controller* this);

void Controller_cleanUpAndExit();

void Controller_forceMovePen(Controller* this, b8 down);

void Controller_penDown(Controller* this);

void Controller_penUp(Controller* this);

/// @return value indicating whether plot should be stopped
///         for now it always return false
b8 Controller_calcPlotter(
	Controller* this,
	Ui*         ui,
	int32_t     moveX,
	int32_t     moveY
);

#endif
