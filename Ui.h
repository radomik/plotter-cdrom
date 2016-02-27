#ifndef PLOTTER_CDROM_UI_H
#define PLOTTER_CDROM_UI_H

#include "common.h"

struct Ui {
	char     message[300];
	uint16_t maxRows;
	uint16_t maxCols;
	uint16_t msgX;
	uint16_t msgY;
};

void Ui_new(Ui* this);

void Ui_initOrExit(Ui* this);

/// Executes the escape sequence. This will move the cursor to x, y
/// Thanks to 'Stack Overflow', found on http://www.daniweb.com/software-development/c/code/216326
void Ui_gotoxy(int x, int y);

///+++++++++++++++++++++++ Start Ui_kbhit ++++++++++++++++++++++++++++++++++
///Thanks to Undertech Blog, http://www.undertec.de/blog/2009/05/kbhit_und_getch_fur_linux.html
int Ui_kbhit();

///+++++++++++++++++++++++ Start Ui_getch ++++++++++++++++++++++++++++++++++
///Thanks to Undertech Blog, http://www.undertec.de/blog/2009/05/kbhit_und_getch_fur_linux.html
int Ui_getch();

/// Clear terminal window
void Ui_clrscr(Ui* this, uint16_t startRow, uint16_t endRow);

void Ui_printRow(Ui* this, char ch, int y);

void Ui_errorText(Ui* this, char *message);

void Ui_errorTextArg(Ui* this, const char* fmt, ...);

void Ui_printMenu01(
	Ui*            this,
	char*          plotFile,
	double         scale,
	double         width,
	double         height,
	uint8_t        moveLength,
	plotter_mode_t plMode
);

void Ui_printMenu02(
	Ui*          this,
	uint16_t     startRow,
	b8           selected,
	char*        fileName,
	size_t       fileNameSize,
	const char*  picturePath
);

void Ui_printMenu03(
	Ui*         this,
	const char* fullFileName,
	uint32_t    numberOfLines,
	long        currentLine,
	long        currentX,
	long        currentY,
	long        startTime
);

void Ui_msg(Ui *this, uint16_t x, uint16_t y, b8 alignment, const char *msg);

void Ui_msgArg(Ui *this, uint16_t x, uint16_t y, b8 alignment, const char *fmt, ...);

#endif
