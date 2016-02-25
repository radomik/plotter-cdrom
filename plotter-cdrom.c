//Project: CD ROM plotter
//Homepage: www.HomoFaciens.de
//Author Norbert Heinz
//Version: 0.5
//Creation date: 10.01.2016
//This program is free software you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation version 3 of the License.
//This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//For a copy of the GNU General Public License see http://www.gnu.org/licenses/
//
//compile with gcc plotter-cdrom.c -I/usr/local/include -L/usr/local/lib -lwiringPi -lm -o plotter-cdrom
//For details see:
//http://www.HomoFaciens.de/technics-machines-plotter-cdrom_en_navion.htm

#include <stdio.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include <dirent.h>
#include <math.h>
#include <wiringPi.h>
#include <unistd.h>
#include <errno.h>

#define SERVOUP     10
#define SERVODOWN   20
#define X_STEPPER01 13
#define X_STEPPER02 14

#define X_STEPPER03 2
#define X_STEPPER04 3

#define X_ENABLE01  12
#define X_ENABLE02  0

#define Y_STEPPER01 6
#define Y_STEPPER02 10

#define Y_STEPPER03 7
#define Y_STEPPER04 9

//#define Y_ENABLE01 6
//#define y_ENABLE02 15

#define STEP_MAX_X  220.0
#define STEP_MAX_Y  220.0

#define Z_SERVO     8

#define BUFFERSIZE  120

static const char* geterr() {
	static char e[64];
	snprintf(e, sizeof(e), "%s", strerror(errno));
	return e;
}

static FILE* LOG_FILE;

static void Log_open() {
	if (! (LOG_FILE = fopen("plotter.log", "wb"))) {
		printf("Error opening log file\n");
	}
}

static void Log_close() {
	if (LOG_FILE) {
		fclose(LOG_FILE);
		LOG_FILE = NULL;
	}
}

#define l_any(level, fmt, ...) if (LOG_FILE) fprintf(LOG_FILE, level fmt "\n", __VA_ARGS__)
#define l_info(fmt, ...)  l_any("[INFO]", fmt, __VA_ARGS__)
#define l_warn(fmt, ...)  l_any("[WARN]", fmt, __VA_ARGS__)
#define l_error(fmt, ...) l_any("[ERRR]", fmt, __VA_ARGS__)

typedef enum plotter_mode_t plotter_mode_t;
enum plotter_mode_t {
	/// Printing bitmap file
	PLOTTER_MODE_PRINT = 0,
	/// Plotting SVG vector graphics file
	PLOTTER_MODE_PLOT
};

typedef struct Bitmap Bitmap;
struct Bitmap {
	unsigned char fileInfo[2];
	uint32_t fileSize;
	uint32_t longTemp;
	uint32_t dataOffset;
	uint32_t headerSize;
	int32_t  pictureWidth;
	int32_t  pictureHeight;
	uint16_t intTemp;
	uint16_t colorDepth;
	uint32_t compressionType;
	uint32_t pictureSize;
	uint32_t xPixelPerMeter;
	uint32_t yPixelPerMeter;
	uint32_t colorNumber;
	uint32_t colorUsed;
};

static int Bitmap_readHeader2(Bitmap* this, FILE* f) {
	if (fread(&this->fileInfo, sizeof(this->fileInfo), 1, f) != 1) return -1;
	if (fread(&this->fileSize, sizeof(this->fileSize), 1, f) != 1) return -1;
	if (fread(&this->longTemp, sizeof(this->longTemp), 1, f) != 1) return -1;
	if (fread(&this->dataOffset, sizeof(this->dataOffset), 1, f) != 1) return -1;
	if (fread(&this->headerSize, sizeof(this->headerSize), 1, f) != 1) return -1;
	if (fread(&this->pictureWidth, sizeof(this->pictureWidth), 1, f) != 1) return -1;
	if (fread(&this->pictureHeight, sizeof(this->pictureHeight), 1, f) != 1) return -1;
	if (fread(&this->intTemp, sizeof(this->intTemp), 1, f) != 1) return -1;
	if (fread(&this->colorDepth, sizeof(this->colorDepth), 1, f) != 1) return -1;
	if (fread(&this->compressionType, sizeof(this->compressionType), 1, f) != 1) return -1;
	if (fread(&this->pictureSize, sizeof(this->pictureSize), 1, f) != 1) return -1;
	if (fread(&this->xPixelPerMeter, sizeof(this->xPixelPerMeter), 1, f) != 1) return -1;
	if (fread(&this->yPixelPerMeter, sizeof(this->yPixelPerMeter), 1, f) != 1) return -1;
	if (fread(&this->colorNumber, sizeof(this->colorNumber), 1, f) != 1) return -1;
	if (fread(&this->colorUsed, sizeof(this->colorUsed), 1, f) != 1) return -1;
	return 0;
}

static int Bitmap_readHeader(Bitmap* this, FILE* f, char* errBuf, size_t errBufSize) {
	if (Bitmap_readHeader2(this, f) != 0) {
		snprintf(errBuf, errBufSize, "Error reading bitmap header [%s]", geterr());
		return -1;
	}
	return 0;
}

static int Bitmap_validateHeader(Bitmap* this, char* errBuf, size_t errBufSize) {
	if (this->fileInfo[0] != 'B' || this->fileInfo[1] != 'M') {
		snprintf(errBuf, errBufSize, "Wrong file info: '%c%c' (expected: 'BM')!", this->fileInfo[0], this->fileInfo[1]);
		return -1;
	}
	if (this->pictureWidth != 55 || this->pictureHeight != 55) {
		snprintf(errBuf, errBufSize, "Wrong picture size: %dx%d (expected 55x55)!", this->pictureWidth, this->pictureHeight);
		return -1;
	}
	if (this->colorDepth != 24) {
		snprintf(errBuf, errBufSize, "Wrong color depth: %hu (expected be 24)!", this->colorDepth);
		return -1;
	}
	if (this->compressionType != 0) {
		snprintf(errBuf, errBufSize, "Wrong compression type: %u (expected 0)!", this->compressionType);
		return -1;
	}
	return 0;
}

static int  MaxRows = 24;
static int  MaxCols = 80;
static int  MessageX = 1;
static int  MessageY = 24;

static int StepX = 0;
static int StepY = 0;
static double StepsPermmX = 250.0 / 35.0;
static double StepsPermmY = 250.0 / 35.0;

static char PicturePath[1000];

/// Executes the escape sequence. This will move the cursor to x, y
/// Thanks to 'Stack Overflow', found on http://www.daniweb.com/software-development/c/code/216326
static void gotoxy(int x, int y) {
	printf("\033[%dd\033[%dG", y, x);
}

/// Clear terminal window
static void clrscr(int startRow, int endRow) {
	int i, j;

	if (endRow < startRow) {
		i = endRow;
		endRow = startRow;
		startRow = i;
	}

	gotoxy(1, startRow);
	endRow -= startRow;

	for (i = 0; i <= endRow; i++) {
		for (j = 0; j < MaxCols; j++) {
			putchar(' ');
		}
		putchar('\n');
	}
}

///+++++++++++++++++++++++ Start kbhit ++++++++++++++++++++++++++++++++++
///Thanks to Undertech Blog, http://www.undertec.de/blog/2009/05/kbhit_und_getch_fur_linux.html
static int kbhit() {
	struct termios term, oterm;
	int fd = 0, c = 0;

	tcgetattr(fd, &oterm);
	term = oterm;

	term.c_lflag = term.c_lflag & (!ICANON);
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
	tcsetattr(fd, TCSANOW, &term);
	c = getchar();
	tcsetattr(fd, TCSANOW, &oterm);

	if (c != -1) {
		ungetc(c, stdin);
	}

	return ((c != -1) ? 1 : 0);
}

///+++++++++++++++++++++++ Start getch ++++++++++++++++++++++++++++++++++
///Thanks to Undertech Blog, http://www.undertec.de/blog/2009/05/kbhit_und_getch_fur_linux.html
static int getch() {
	struct termios new, old;
	static int ch, fd;

	fd = fileno(stdin);
	tcgetattr(fd, &old);
	new = old;
	new.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(fd, TCSANOW, &new);
	ch = getchar();
	tcsetattr(fd, TCSANOW, &old);

	return ch;
}

//++++++++++++++++++++++ Start MessageText +++++++++++++++++++++++++++++
static void Msg(int x, int y, int alignment, char *message) {
	clrscr(y, y);
	gotoxy(x, y);

	if (alignment) {
		int i;
		y = (MaxCols - strlen(message)) / 2;

		for (i = 0; i < y; i++) {
			putchar(' ');
		}
	}
	puts(message);
}

static void MsgArg(int x, int y, int alignment, const char *fmt, ...) {
	static char message[300];
	va_list va;
	va_start(va, fmt);
	vsnprintf(message, sizeof(message), fmt, va);
	va_end(va);
	Msg(x, y, alignment, message);
}

///++++++++++++++++++++++ Start PrintRow ++++++++++++++++++++++++++++++++
static void PrintRow(char ch, int y) {
	int i;
	gotoxy(1, y);
	for (i = 0; i < MaxCols; i++) {
		putchar(ch);
	}
}

///+++++++++++++++++++++++++ ErrorText +++++++++++++++++++++++++++++
static void ErrorText(char *message) {
	int y = MessageY + 2;
	clrscr(y, y);
	gotoxy(1, y);
	printf("Last error: %s", message);
}

static void ErrorTextArg(const char* fmt, ...) {
	static char message[300];
	va_list va;
	va_start(va, fmt);
	vsnprintf(message, sizeof(message), fmt, va);
	va_end(va);
	ErrorText(message);
}

///+++++++++++++++++++++++++ PrintMenue_01 ++++++++++++++++++++++++++++++
static void PrintMenue_01(char* PlotFile, double scale, double width, double height, long MoveLength, plotter_mode_t plMode) {
	double s2 = scale / 10.0 / StepsPermmX;
	double w  = width  * s2;
	double h  = height * s2;

	clrscr(1, MessageY-2);
	Msg(1, 1, 1,      "*** Main menu plotter ***");
	MsgArg(10, 3, 0,  "M            - toggle move length, current value = %ld step(s)", MoveLength);
	Msg(10, 4, 0,     "Cursor right - move plotter in positive X direction");
	Msg(10, 5, 0,     "Cursor left  - move plotter in negative X direction");
	Msg(10, 6, 0,     "Cursor up    - move plotter in positive Y direction");
	Msg(10, 7, 0,     "Cursor down  - move plotter in negative Y direction");
	Msg(10, 8, 0,     "Page up      - lift pen");
	Msg(10, 9, 0,     "Page down    - touch down pen");
	MsgArg(10, 10, 0, "F            - choose file. Current file = \"%s\"", PlotFile);
	MsgArg(10, 11, 0, "               Scale set to = %0.4f. W = %0.2fcm, H = %0.2fcm", scale, w, h);
	Msg(10, 12, 0,    "P            - plot file");

	if (plMode == PLOTTER_MODE_PRINT) {
		Msg(10, 13, 0, "               Operating mode: PRINTING");
	}
	else {
		if (plMode == PLOTTER_MODE_PLOT) {
			Msg(10, 13, 0, "               Operating mode: PLOTTING");
		}
	}

	Msg(10, 16, 0, "Esc          - leave program");
}

///+++++++++++++++++++++++++ PrintMenue_02 ++++++++++++++++++++++++++++++
static char* PrintMenue_02(int startRow, int selected) {
	static char fileName[256];
	DIR *pDIR;
	struct dirent *pDirEnt;
	int i = 0;
	int discard = 0;
	char cSel[2];

	clrscr(1, MessageY-2);
	Msg(1, 1, 1, "*** Choose plotter file ***");

	if (! (pDIR = opendir(PicturePath))) {
		MsgArg(1, 4, 1, "Could not open directory '%s'!", PicturePath);
		getch();
		return "";
	}

	while ( ((pDirEnt = readdir(pDIR)) != NULL) && (i < 10) ) {
		if (strlen(pDirEnt->d_name) > 4) {
			if ((! memcmp(pDirEnt->d_name + strlen(pDirEnt->d_name)-4, ".svg", 4)) ||
				(! memcmp(pDirEnt->d_name + strlen(pDirEnt->d_name)-4, ".bmp", 4)))
			{
				if (discard >= startRow) {
					if (i + startRow == selected) {
						cSel[0] = '>';
						cSel[1] = '<';
						snprintf(fileName, sizeof(fileName), "%s", pDirEnt->d_name);
					}
					else {
						cSel[0] = cSel[1] = ' ';
					}
					MsgArg(1, 3 + i, 0, "%c%s%c", cSel[0], pDirEnt->d_name, cSel[1]);
					i++;
				}

				discard++;
			}
		}
	}
	gotoxy(MessageX, MessageY + 1);
	printf("Choose file using up/down keys and confirm with 'Enter' or press 'Esc' to cancel.");

	return fileName;
}

///+++++++++++++++++++++++++ PrintMenue_03 ++++++++++++++++++++++++++++++
static void PrintMenue_03(char *fullFileName, long numberOfLines, long currentLine, long currentX, long currentY, long startTime) {
	unsigned char processHours = 0, processMinutes = 0, processSeconds;
	long currentTime = time(0) - startTime;

	while (currentTime > 3600) {
		processHours++;
		currentTime -= 3600;
	}
	while (currentTime > 60) {
		processMinutes++;
		currentTime -= 60;
	}

	processSeconds = currentTime;

	clrscr(1, MessageY - 2);
	Msg(1, 1, 1,     "*** Plotting file ***");
	MsgArg(10, 3, 0, "File name: %s", fullFileName);
	MsgArg(10, 4, 0, "Number of lines: %ld", numberOfLines);
	MsgArg(10, 5, 0, "Current Position(%ld): X = %ld, Y = %ld     ", currentLine, currentX, currentY);
	MsgArg(10, 6, 0, "Process time: %02u:%02u:%02u", processHours, processMinutes, processSeconds);
}

///++++++++++++++++++++++++++++++ MakeStepX ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void MakeStepX(int direction, long stepPause) {
	StepX += direction;

	if (StepX > 3) {
		StepX = 0;
	}
	else {
		if (StepX < 0) {
			StepX = 3;
		}
	}

	// You might have to swap the sequence of steps!!!
	// If your motor doesn't rotate as expected, try:
	//  StepX == 0   StepX == 1   StepX == 2   StepX == 3
	//  1 0 0 0      0 0 0 1      0 1 0 0      0 0 1 0
	// or:
	//  StepX == 0   StepX == 1   StepX == 2   StepX == 3
	//  1 0 0 0      0 0 1 0      0 1 0 0      0 0 0 1
	// or:
	//  StepX == 0   StepX == 1   StepX == 2   StepX == 3
	//  1 0 0 0      0 1 0 0      0 0 1 0      0 0 0 1
	// or:
	//  StepX == 0   StepX == 1   StepX == 2   StepX == 3
	//  1 0 0 0      0 1 0 0      0 0 0 1      0 0 1 0

	switch (StepX) {
		case 0:
			digitalWrite(X_STEPPER01, 1);
			digitalWrite(X_STEPPER02, 0);
			digitalWrite(X_STEPPER03, 0);
			digitalWrite(X_STEPPER04, 0);
			break;
		case 1:
			digitalWrite(X_STEPPER01, 0);
			digitalWrite(X_STEPPER02, 0);
			digitalWrite(X_STEPPER03, 1);
			digitalWrite(X_STEPPER04, 0);
			break;
		case 2:
			digitalWrite(X_STEPPER01, 0);
			digitalWrite(X_STEPPER02, 1);
			digitalWrite(X_STEPPER03, 0);
			digitalWrite(X_STEPPER04, 0);
			break;
		case 3:
			digitalWrite(X_STEPPER01, 0);
			digitalWrite(X_STEPPER02, 0);
			digitalWrite(X_STEPPER03, 0);
			digitalWrite(X_STEPPER04, 1);
			break;
	}

	usleep(stepPause);
}

//++++++++++++++++++++++++++++++ MakeStepY ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void MakeStepY(int direction, long stepPause) {
	StepY += direction;

	if (StepY > 3) {
		StepY = 0;
	}
	else {
		if (StepY < 0) {
			StepY = 3;
		}
	}

	// You might have to swap the sequence of steps!!!
	// If your motor doesn't rotate as expected, try:
	//  StepY == 0   StepY == 1   StepY == 2   StepY == 3
	//  1 0 0 0      0 0 0 1      0 1 0 0      0 0 1 0
	// or:
	//  StepY == 0   StepY == 1   StepY == 2   StepY == 3
	//  1 0 0 0      0 0 1 0      0 1 0 0      0 0 0 1
	// or:
	//  StepY == 0   StepY == 1   StepY == 2   StepY == 3
	//  1 0 0 0      0 1 0 0      0 0 1 0      0 0 0 1
	// or:
	//  StepY == 0   StepY == 1   StepY == 2   StepY == 3
	//  1 0 0 0      0 1 0 0      0 0 0 1      0 0 1 0

	switch (StepY) {
		case 0:
			digitalWrite(Y_STEPPER01, 1);
			digitalWrite(Y_STEPPER02, 0);
			digitalWrite(Y_STEPPER03, 0);
			digitalWrite(Y_STEPPER04, 0);
			break;
		case 1:
			digitalWrite(Y_STEPPER01, 0);
			digitalWrite(Y_STEPPER02, 0);
			digitalWrite(Y_STEPPER03, 1);
			digitalWrite(Y_STEPPER04, 0);
			break;
		case 2:
			digitalWrite(Y_STEPPER01, 0);
			digitalWrite(Y_STEPPER02, 1);
			digitalWrite(Y_STEPPER03, 0);
			digitalWrite(Y_STEPPER04, 0);
			break;
		case 3:
			digitalWrite(Y_STEPPER01, 0);
			digitalWrite(Y_STEPPER02, 0);
			digitalWrite(Y_STEPPER03, 0);
			digitalWrite(Y_STEPPER04, 1);
			break;
	}

	usleep(stepPause);
}

///++++++++++++++++++++++++++++++++++++++ CalculatePlotter ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int CalculatePlotter(long moveX, long moveY, long stepPause) {
	long tempX = 0, tempY = 0;
	int  i = 0;

	MsgArg(MessageX, MessageY, 0, "Moving X: %ld, Moving Y: %ld", moveX, moveY);

	if (moveX == 0) {
		if (moveY > 0) {
			for (i = 0; i < moveY; i++) {
				MakeStepY(-1, stepPause);
			}
		}
		else {
			if (moveY < 0) {
				for (i = 0; i < -moveY; i++) {
					MakeStepY(1, stepPause);
				}
			}
		}
	}

	if (moveY == 0) {
		if (moveX > 0) {
			for (i = 0; i < moveX; i++) {
				MakeStepX(1, stepPause);
			}
		}
		else {
			if (moveX < 0) {
				for (i = 0; i < -moveX; i++) {
					MakeStepX(-1, stepPause);
				}
			}
		}
	}

	if (moveY != 0 && moveX != 0) {
		if (abs(moveX) > abs(moveY)) {
			while (moveY != 0) {
				tempX = moveX / abs(moveY);
				if (tempX == 0) {
					printf("tempX=%ld, moveX=%ld, moveY=%ld    \n", tempX, moveX, moveY);
				}
				else {
					if (tempX > 0) {
						for (i = 0; i < tempX; i++) {
							MakeStepX(1, stepPause);
						}
					}
					else { // tempX < 0
						for (i = 0; i < -tempX; i++) {
							MakeStepX(-1, stepPause);
						}
					}
				}

				moveX -= tempX;
				if (moveY > 0) {
					MakeStepY(-1, stepPause);
					moveY--;
				}
				else {
					if (moveY < 0) {
						MakeStepY(1, stepPause);
						moveY++;
					}
				}
			} // while (moveY != 0)

			// move remaining X coordinates
			if (moveX > 0) {
				for (i = 0; i < moveX; i++) {
					MakeStepX(1, stepPause);
				}
			}
			else {
				if (moveX < 0) {
					for (i = 0; i < -moveX; i++) {
						MakeStepX(-1, stepPause);
					}
				}
			}
		}
		else { // abs(moveX) <= abs(moveY)
			while (moveX != 0) {
				tempY = moveY / abs(moveX);
				if (tempY == 0) {
					printf("tempY=%ld, moveX=%ld, moveY=%ld    \n", tempX, moveX, moveY);
				}
				else {
					if (tempY > 0) {
						for (i = 0; i < tempY; i++) {
							MakeStepY(-1, stepPause);
						}
					}
					else { // tempY < 0
						for (i = 0; i < -tempY; i++) {
							MakeStepY(1, stepPause);
						}
					}
				}

				moveY -= tempY;
				if (moveX > 0) {
					MakeStepX(1, stepPause);
					moveX--;
				}
				else {
					if (moveX < 0) {
						MakeStepX(-1, stepPause);
						moveX++;
					}
				}
			}

			// move remaining Y coordinates
			if (moveY > 0) {
				for (i = 0; i < moveY; i++) {
					MakeStepY(-1, stepPause);
				}
			}
			else {
				if (moveY < 0) {
					for (i = 0; i < -moveY; i++) {
						MakeStepY(1, stepPause);
					}
				}
			}
		}
	}

	return 0;
}

//######################################################################
//################## Main ##############################################
//######################################################################

static unsigned char currentPlotDown = 0;

#define PEN_FORCE_UP()					\
{										\
	softPwmWrite(Z_SERVO, SERVOUP);		\
	usleep(500000);						\
	softPwmWrite(Z_SERVO, 0);			\
	currentPlotDown = 0;				\
}

#define PEN_FORCE_DOWN()				\
{										\
	softPwmWrite(Z_SERVO, SERVODOWN);	\
	usleep(500000);						\
	softPwmWrite(Z_SERVO, 0);			\
	currentPlotDown = 1;				\
}

#define PEN_UP()							\
	if (currentPlotDown) PEN_FORCE_UP()

#define PEN_DOWN()							\
	if (! currentPlotDown) PEN_FORCE_DOWN()

static void gpioInitOrExit() {
	if (wiringPiSetup() == -1) {
		printf("Could not run wiringPiSetup! [%s]\n", geterr());
		exit(1);
	}

	softPwmCreate(Z_SERVO, SERVOUP, 200);
	PEN_FORCE_UP();

	pinMode(X_STEPPER01, OUTPUT);
	pinMode(X_STEPPER02, OUTPUT);
	pinMode(X_STEPPER03, OUTPUT);
	pinMode(X_STEPPER04, OUTPUT);
	pinMode(X_ENABLE01, OUTPUT);
	pinMode(X_ENABLE02, OUTPUT);

	digitalWrite(X_STEPPER01, 1);
	digitalWrite(X_STEPPER02, 0);
	digitalWrite(X_STEPPER03, 0);
	digitalWrite(X_STEPPER04, 0);

	digitalWrite(X_ENABLE01, 1);
	digitalWrite(X_ENABLE02, 1);

	pinMode(Y_STEPPER01, OUTPUT);
	pinMode(Y_STEPPER02, OUTPUT);
	pinMode(Y_STEPPER03, OUTPUT);
	pinMode(Y_STEPPER04, OUTPUT);
	digitalWrite(Y_STEPPER01, 1);
	digitalWrite(Y_STEPPER02, 0);
	digitalWrite(Y_STEPPER03, 0);
	digitalWrite(Y_STEPPER04, 0);
}

static void gpioCleanupAndExit() {
	digitalWrite(X_STEPPER01, 0);
	digitalWrite(X_STEPPER02, 0);
	digitalWrite(X_STEPPER03, 0);
	digitalWrite(X_STEPPER04, 0);
	digitalWrite(X_ENABLE01, 0);
	digitalWrite(X_ENABLE02, 0);

	digitalWrite(Y_STEPPER01, 0);
	digitalWrite(Y_STEPPER02, 0);
	digitalWrite(Y_STEPPER03, 0);
	digitalWrite(Y_STEPPER04, 0);
	exit(0);
}

static void exiting() {
	Log_close();
}

int main(int argc, char **argv) {
	Bitmap bmp;
	int menuLevel = 0;
	int keyHit = 0;
	int keyCode[5];
	char fileName[200] = "";
	char fullFileName[200] = "";
	char fileNameOld[200] = "";
	struct winsize terminal;
	double scale = 1.0;
	long moveLength = 1;
	plotter_mode_t plMode = PLOTTER_MODE_PRINT;
	int i;
	unsigned char singleKey=0;
	long stepPause = 10000;
	long currentPlotX = 0, currentPlotY = 0;
	int fileSelected = 0;
	int fileStartRow = 0;
	char *pEnd;
	FILE *plotFile;
	char textLine[300];
	long xMin = 1000000, xMax = -1000000;
	long yMin = 1000000, yMax = -1000000;
	long coordinateCount = 0;
	char a;
	int rdState = 0;
	long xNow = 0, yNow = 0;
	long xNow1 = 0, yNow1 = 0;
	long xNow2 = 0, yNow2 = 0;
	long stepsPerPixelX = (double)(STEP_MAX_X) / 55.0, StepsPerPixelY = (double)STEP_MAX_Y / 55.0;
	long fillBytes = 0;
	long coordinatePlot = 0;
	int stopPlot = 0;
	int reverseMode, newLine;
	unsigned char bgr[3], bgrNext[3];
	long plotStartTime = 0;

	atexit(exiting);
	Log_open();

	strcpy(fileName, "noFiLE");

	if (argc > 1) {
		if (! (strcmp(argv[1], "--help"))) {
			printf("Usage:\n\
	%s [PICTURES_PATH]\n\n\
	Where PICTURES_PATH is optional argument with path to directory \n\
	containing SVG and/or BMP files. Default is: ./pictures\n", argv[0]);
			exit(0);
		}
		snprintf(PicturePath, sizeof(PicturePath), "%s", argv[1]);
	}
	else {
		getcwd(PicturePath, 1000);
		strcat(PicturePath, "/pictures");
	}

	printf("PicturePath=>%s<", PicturePath);
	l_info("Picture path: '%s'", PicturePath);

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal) < 0) {
		printf("Can't get size of terminal window [%s]\n", geterr());
		exit(1);
	}
	else {
		MaxRows = terminal.ws_row;
		MaxCols = terminal.ws_col;
		MessageY = MaxRows-3;
	}

	l_info("Terminal size rows x cols: %d x %d", MaxRows, MaxCols);

	gpioInitOrExit();

	clrscr(1, MaxRows);
	PrintRow('-', MessageY - 1);
	PrintMenue_01(fileName, scale, xMax - xMin, yMax - yMin, moveLength, plMode);

	while (1) {
		Msg(MessageX, MessageY, 0, "Waiting for key press...");

		i = 0;
		singleKey = 1;
		memset(keyCode, 0, sizeof(keyCode));
		keyHit = 0;

		while (kbhit()) {
			keyHit = getch();
			keyCode[i++] = keyHit;
			if (i == 5) {
				i = 0;
			}
			else {
				if (i > 1) {
					singleKey = 0;
				}
			}
		}

		if (! singleKey) {
			keyHit = 0;
		}

		if (menuLevel == 0) {

			// Move X-axis
			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 68 && keyCode[3] == 0 && keyCode[4] == 0) {
				CalculatePlotter(-moveLength, 0, stepPause);
			}

			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 67 && keyCode[3] == 0 && keyCode[4] == 0) {
				CalculatePlotter(moveLength, 0, stepPause);
			}

			// Move Y-axis
			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 65 && keyCode[3] == 0 && keyCode[4] == 0) {
				CalculatePlotter(0, moveLength, stepPause);
			}

			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 66 && keyCode[3] == 0 && keyCode[4] == 0) {
				CalculatePlotter(0, -moveLength, stepPause);
			}

			// Pen UP/DOWN
			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 53 && keyCode[3] == 126 && keyCode[4] == 0) {
				PEN_FORCE_UP();
			}

			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 54 && keyCode[3] == 126 && keyCode[4] == 0) {
				PEN_FORCE_DOWN();
			}


			if (keyHit == 'm') {
				if (moveLength == 1) {
					moveLength = 10;
				}
				else {
					moveLength = 1;
				}
				PrintMenue_01(fileName, scale, xMax - xMin, yMax - yMin, moveLength, plMode);
			}

			if (keyHit == 'f') {
				fileStartRow = 0;
				fileSelected = 0;
				strcpy(fileNameOld, fileName);
				strcpy(fileName, PrintMenue_02(fileStartRow, 0));
				menuLevel = 1;
			}

			if (keyHit == 'p') { // Plot file
				Msg(1, 20, 0, "3 seconds until plotting starts !!!!!!!!!!!!!!!!!");
				sleep(3);
				if (strcmp(fileName, "noFiLE") != 0) {
					if (! (plotFile=fopen(fullFileName, "rb"))) {
						sprintf(textLine, "Can't open file '%s'!\n", fullFileName);
						strcpy(fileName, "NoFiLE");
						ErrorText(textLine);
					}
				}
				if (strcmp(fileName, "noFiLE") != 0) {
					if (plMode == PLOTTER_MODE_PLOT) { // Plot SVG file
						xNow1 = -1;
						xNow2 = -1;
						yNow1 = -1;
						yNow2 = -1;

						currentPlotX = 0;
						currentPlotY = 0;
						coordinatePlot = 0;
						stopPlot = 0;

						plotStartTime = time(0);

						PrintMenue_03(fullFileName, coordinateCount, 0, 0, 0, plotStartTime);

						PEN_UP();

						while (!(feof(plotFile)) && stopPlot == 0) {

							fread(&a, 1, 1, plotFile);
							i = 0;
							textLine[0] = '\0';

							while (a != ' ' && a != '<' && a != '>' && a != '\"' && a != '=' && a != ',' && a != ':') {
								textLine[i] = a;
								textLine[++i] = '\0';
								fread(&a, 1, 1, plotFile);
							}

							if (a == '<') { // Init
								if (xNow2 > -1 && yNow2 > -1 && (xNow2 != xNow1 || yNow2 != yNow1)) {
									stopPlot = CalculatePlotter(xNow2 - currentPlotX, yNow2 - currentPlotY, stepPause);
									PEN_DOWN();

									currentPlotX = xNow2;
									currentPlotY = yNow2;

									stopPlot = CalculatePlotter(xNow1 - currentPlotX, yNow1 - currentPlotY, stepPause);
									currentPlotX = xNow1;
									currentPlotY = yNow1;

									stopPlot = CalculatePlotter(xNow - currentPlotX, yNow - currentPlotY, stepPause);
									currentPlotX = xNow;
									currentPlotY = yNow;
								}
								rdState = 0;
								xNow1 = -1;
								xNow2 = -1;
								yNow1 = -1;
								yNow2 = -1;
							}

							if (strcmp(textLine, "path") == 0) {
								PEN_UP();
								rdState = 1; // path found
							}
							if (rdState == 1 && strcmp(textLine, "fill") == 0) {
								rdState = 2; // fill found
							}
							if (rdState == 2 && strcmp(textLine, "none") == 0) {
								rdState = 3; // none found
							}
							if (rdState == 2 && strcmp(textLine, "stroke") == 0) {
								rdState = 0; // stroke found, fill isn't "none"
							}
							if (rdState == 3 && strcmp(textLine, "d") == 0 && a == '=') {
								rdState = 4; // d= found
							}
							if (rdState == 4 && strcmp(textLine, "M") == 0 && a == ' ') {
								rdState = 5; // M found
							}

							if (rdState == 6) { // Y value
								yNow = (strtol(textLine, &pEnd, 10) - yMin) * scale * StepsPermmY / StepsPermmX;
								rdState = 7;
							}
							if (rdState == 5 && a == ',') { // X value
								xNow = ((xMax - strtol(textLine, &pEnd, 10))) * scale;
								rdState = 6;
							}
							if (rdState == 7) {
								if (xNow2 > -1 && yNow2 > -1 && (xNow2 != xNow1 || yNow2 != yNow1)) {
									stopPlot = CalculatePlotter(xNow2 - currentPlotX, yNow2 - currentPlotY, stepPause);
									PEN_DOWN();
									currentPlotX = xNow2;
									currentPlotY = yNow2;
								}
								xNow2 = xNow1;
								yNow2 = yNow1;
								xNow1 = xNow;
								yNow1 = yNow;
								rdState = 5;
							}
						} // END: while (!(feof(plotFile)) && stopPlot == 0)

						fclose(plotFile);

						PEN_UP();

						PrintMenue_03(fullFileName, coordinateCount, coordinatePlot, 0, 0, plotStartTime);
						CalculatePlotter( -currentPlotX, -currentPlotY, stepPause );
						currentPlotX = currentPlotY = 0;

						while (kbhit()) {
							getch();
						}
						Msg(MessageX, MessageY, 0, "Finished! Press any key to return to main menu.");
						getch();
						PrintMenue_01(fileName, scale, xMax - xMin, yMax - yMin, moveLength, plMode);
					} // if (pl == PLOTTER_MODE_PLOT)

					if (plMode == PLOTTER_MODE_PRINT) { // bitmap
						if (Bitmap_readHeader(&bmp, plotFile, textLine, sizeof(textLine)) != 0) {
							l_error("Error reading bitmap header of file '%s' [%s]", fullFileName, textLine);
							// TODO: Not ignore this error
						}

						fillBytes = 0;
						while ((bmp.pictureWidth * 3 + fillBytes) % 4 != 0) {
							fillBytes++;
						}
						CalculatePlotter( 0, stepsPerPixelX * bmp.pictureWidth, stepPause );
						fseek(plotFile, bmp.dataOffset, SEEK_SET);
						reverseMode = 0;

						for (currentPlotY = 0; currentPlotY < bmp.pictureHeight; currentPlotY++) {
							newLine = 0;
							currentPlotX = (reverseMode) ? (bmp.pictureWidth - 1) : 0;

							while (newLine == 0) {
								fseek(plotFile, bmp.dataOffset + (currentPlotX * bmp.pictureWidth + currentPlotX) * 3 + fillBytes * currentPlotY, SEEK_SET);
								fread(&bgr, sizeof(bgr), 1, plotFile);

								if (reverseMode == 1) {
									fseek(plotFile, bmp.dataOffset + (currentPlotX * bmp.pictureWidth + currentPlotX - 1) * 3 + fillBytes * currentPlotY, SEEK_SET);
								}

								fread(&bgrNext, sizeof(bgrNext), 1, plotFile);

								if (bgr[2] < 200 || bgr[1] < 200 || bgr[0] < 200) {
									PEN_DOWN();

									if (bgrNext[2] > 199 && bgrNext[1] > 199 && bgrNext[0] > 199) {
										PEN_UP();
									}
								}
								else {
									PEN_UP();
								}

								if (reverseMode == 0) {
									currentPlotX++;
									if (currentPlotX < bmp.pictureWidth) {
										CalculatePlotter(stepsPerPixelX, 0, stepPause); // X-Y movement swapped!!!
									}
									else {
										newLine = 1;
										reverseMode = 1;
									}
								}
								else {
									currentPlotX--;
									if (currentPlotX > -1) {
										CalculatePlotter(-stepsPerPixelX, 0, stepPause); // X-Y movement swapped!!!
									}
									else {
										newLine = 1;
										reverseMode = 0;
									}
								}
							} // while (newLine == 0)

							PEN_UP();

							CalculatePlotter( 0, -StepsPerPixelY, stepPause );
						} // for (currentPlotY = 0; currentPlotY < bmp.pictureHeight + jetOffset1 + jetOffset2; currentPlotY++)

						fclose(plotFile);
						PEN_UP();

						if (reverseMode == 1) {
							CalculatePlotter( -stepsPerPixelX * bmp.pictureWidth, 0, stepPause );
						}
					} // if (plMode == PLOTTER_MODE_PRINT)
				} // if (strcmp(fileName, "noFiLE") != 0)
			} // if (keyHit == 'p')
		} // if (menuLevel == 0)

		if (menuLevel == 1) { // Select file

			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 66 && keyCode[3] == 0 && keyCode[4] == 0) {
				fileSelected++;
				strcpy(fileName, PrintMenue_02(fileStartRow, fileSelected));
			}

			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[2] == 65 && keyCode[3] == 0 && keyCode[4] == 0) {
				if (fileSelected > 0) {
					fileSelected--;
					strcpy(fileName, PrintMenue_02(fileStartRow, fileSelected));
				}
			}

			if (keyHit == 10) { // Read file and store values
				menuLevel = 0;
				clrscr(MessageY + 1, MessageY + 1);
				strcpy(fullFileName, PicturePath);
				strcat(fullFileName, "/");
				strcat(fullFileName, fileName);
				if (! (plotFile=fopen(fullFileName,"rb"))) {
					sprintf(textLine, "Can't open file '%s'!\n", fullFileName);
					ErrorText(textLine);
					ErrorTextArg("Can't open file '%s' [%s]!\n", fullFileName, geterr());
					strcpy(fileName, "NoFiLE");
				}
				else {
					if (memcmp(fileName + strlen(fileName)-4, ".svg", 4) == 0) {
						plMode = PLOTTER_MODE_PLOT;
					}
					else {
						plMode = PLOTTER_MODE_PRINT;
					}
					xMin = yMin =  1000000;
					xMax = yMax = -1000000;
					coordinateCount = 0;

					if (plMode == PLOTTER_MODE_PLOT) {
						while (!(feof(plotFile)) && stopPlot == 0) {
							fread(&a, 1, 1, plotFile);
							i = 0;
							textLine[0] = '\0';
							while (a !=' ' && a != '<' && a != '>' && a != '\"' && a != '=' && a != ',' && a != ':') {
								textLine[i] = a;
								textLine[++i] = '\0';
								fread(&a, 1, 1, plotFile);
							}
							if (a == '<') { // Init
								rdState = 0;
							}
							if (strcmp(textLine, "path") == 0) {
								rdState = 1; // path found
							}
							if (rdState == 1 && strcmp(textLine, "fill") == 0) {
								rdState = 2;//fill found
							}
							if (rdState == 2 && strcmp(textLine, "none") == 0) {
								rdState = 3; // none found
							}
							if (rdState == 2 && strcmp(textLine, "stroke") == 0) {
								rdState = 0; // stroke found, fill isn't "none"
							}
							if (rdState == 3 && strcmp(textLine, "d") == 0 && a == '=') {
								rdState = 4; // d= found
							}
							if (rdState == 4 && strcmp(textLine, "M") == 0 && a == ' ') {
								rdState = 5; // M found
							}
							if (rdState == 5 && strcmp(textLine, "C") == 0 && a == ' ') {
								rdState = 5; // C found
							}

							if (rdState == 6) { // Y value
								yNow = strtol(textLine, &pEnd, 10);
								//printf("String='%s' y=%ld\n", textLine, yNow);
								if (yNow > yMax) {
									yMax = yNow;
								}
								else {
									if (yNow < yMin) {
										yMin = yNow;
									}
								}
								rdState = 7;
							}

							if (rdState == 5 && a == ',') { // X value
								xNow = strtol(textLine, &pEnd, 10);
								if (xNow > xMax) {
									xMax = xNow;
								}
								else {
									if (xNow < xMin) {
										xMin = xNow;
									}
								}
								rdState = 6;
							}

							if (rdState == 7) {
								//printf("Found coordinates %ld, %ld\n", xNow, yNow);
								rdState = 5;
							}

							gotoxy(1, MessageY);
							printf("rdState=% 3d, xNow=% 10ld, xMin=% 10ld, xMax=% 10ld, yMin=% 10ld, yMax=% 10ld   ",
								rdState, xNow, xMin, xMax, yMin, yMax);

						} // while (!(feof(plotFile)) && stopPlot == 0)

						fclose(plotFile);
						if (xMax - xMin > yMax -yMin) {
							scale = STEP_MAX_X / (double)(xMax - xMin);
						}
						else {
							scale = STEP_MAX_X / (double)(yMax - yMin);
						}

						//getch();
					} // if (plMode == PLOTTER_MODE_PLOT)

					if (plMode == PLOTTER_MODE_PRINT) { // bitmap
						if ((Bitmap_readHeader(&bmp, plotFile, textLine, sizeof(textLine)) != 0) ||
							(Bitmap_validateHeader(&bmp, textLine, sizeof(textLine)) != 0))
						{
							ErrorText(textLine);
							strcpy(fileName, "NoFiLE");
						}

						xMin = yMin = 0;
						xMax = bmp.pictureWidth * stepsPerPixelX;
						yMax = bmp.pictureHeight * StepsPerPixelY;
						coordinateCount = bmp.pictureWidth * bmp.pictureHeight;
						scale = 1.0;
					}
				}

				//picWidth = (double)(xMax - xMin) * scale;
				//picHeight = (double)(yMax - yMin) * scale;
				PrintMenue_01(fileName, scale, xMax - xMin, yMax - yMin, moveLength, plMode);
			} // if (keyHit == 10)

		} // if (menuLevel == 1)


		if (keyHit == 27) {
			if (menuLevel == 0) {
				clrscr(MessageY + 1, MessageY + 1);
				MessageText("Exit program (y/n)?", MessageX, MessageY + 1, 0);
				while (keyHit != 'y' && keyHit != 'n') {
					keyHit = getch();
					if (keyHit == 'y' || keyHit == 'Y') {
						gpioCleanupAndExit();
					}
				}
			}

			if (menuLevel == 1) {
				menuLevel = 0;
				strcpy(fileName, fileNameOld);
				PrintMenue_01(fileName, scale, xMax - xMin, yMax - yMin, moveLength, plMode);
			}
			clrscr(MessageY + 1, MessageY + 1);
		}
	}

	return 0;
}
