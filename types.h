#ifndef PLOTTER_CDROM_TYPES_H
#define PLOTTER_CDROM_TYPES_H

#include <inttypes.h>

typedef uint8_t b8;

#ifdef true
	#undef true
#endif
#ifdef false
	#undef false
#endif
#define true 1
#define false 0

typedef enum   plotter_mode_t plotter_mode_t;
typedef enum   menu_level_t menu_level_t;
typedef struct Bitmap Bitmap;
typedef struct Controller Controller;
typedef struct Log Log;
typedef struct Ui Ui;
typedef struct Vars Vars;

#endif
