#ifndef PLOTTER_CDROM_COMMON_H
#define PLOTTER_CDROM_COMMON_H

#include "types.h"
#include "Log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>

/// relative to current working directory
#define DEFAULT_PICTURE_PATH "/pictures"

#define STEP_MAX_X  220.0
#define STEP_MAX_Y  220.0

/// in pixels
#define BITMAP_MAX_WIDTH  55
#define BITMAP_MAX_HEIGHT 55

/// in microseconds
#define STEP_PAUSE 10000

/// in microseconds
#define SERVO_PAUSE 500000

#define STEPS_PERMM_X 250.0 / 35.0
#define STEPS_PERMM_Y 250.0 / 35.0


enum plotter_mode_t {
	/// Printing bitmap file
	PLOTTER_MODE_PRINT = 0,
	/// Plotting SVG vector graphics file
	PLOTTER_MODE_PLOT
};

enum menu_level_t {
	/// Main menu
	MENU_LEVEL_MAIN = 0,
	/// Menu choosing file to print
	MENU_LEVEL_FILE_CHOOSER
};

#endif
