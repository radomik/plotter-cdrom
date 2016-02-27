#include "Ui.h"
#include <stdarg.h>
#include <dirent.h>
#include <termios.h>
#include <sys/ioctl.h>

void Ui_new(Ui* this) {
	this->maxRows = 24;
	this->maxCols = 80;
	this->msgX    = 1;
	this->msgY    = 24;
}

void Ui_initOrExit(Ui* this) {
	struct winsize terminal;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal) < 0) {
		fprintf(stderr, "Can't get size of terminal window [%s]\n", geterr());
		exit(1);
	}
	else {
		this->maxRows = terminal.ws_row;
		this->maxCols = terminal.ws_col;
		this->msgY = this->maxRows-3;
	}

	l_info("Terminal size rows x cols: %d x %d", this->maxRows, this->maxCols);
}

/// Executes the escape sequence. This will move the cursor to x, y
/// Thanks to 'Stack Overflow', found on http://www.daniweb.com/software-development/c/code/216326
void Ui_gotoxy(int x, int y) {
	printf("\033[%dd\033[%dG", y, x);
}

///+++++++++++++++++++++++ Start Ui_kbhit ++++++++++++++++++++++++++++++++++
///Thanks to Undertech Blog, http://www.undertec.de/blog/2009/05/kbhit_und_getch_fur_linux.html
int Ui_kbhit() {
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

///+++++++++++++++++++++++ Start Ui_getch ++++++++++++++++++++++++++++++++++
///Thanks to Undertech Blog, http://www.undertec.de/blog/2009/05/kbhit_und_getch_fur_linux.html
int Ui_getch() {
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

/// Clear terminal window
void Ui_clrscr(Ui* this, uint16_t startRow, uint16_t endRow) {
	uint16_t i, j;

	if (endRow < startRow) {
		i = endRow;
		endRow = startRow;
		startRow = i;
	}

	Ui_gotoxy(1, startRow);
	endRow -= startRow;

	for (i = 0; i <= endRow; i++) {
		for (j = 0; j < this->maxCols; j++) {
			putchar(' ');
		}
		putchar('\n');
	}
}

///++++++++++++++++++++++ Start Ui_printRow ++++++++++++++++++++++++++++++++
void Ui_printRow(Ui* this, char ch, int y) {
	uint16_t i;
	Ui_gotoxy(1, y);
	for (i = 0; i < this->maxCols; i++) {
		putchar(ch);
	}
}

///+++++++++++++++++++++++++ Ui_errorText +++++++++++++++++++++++++++++
void Ui_errorText(Ui* this, char *message) {
	uint16_t y = this->msgY + 2;
	Ui_clrscr(this, y, y);
	Ui_gotoxy(1, y);
	printf("Last error: %s", message);
}

void Ui_errorTextArg(Ui* this, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vsnprintf(this->message, sizeof(this->message), fmt, va);
	va_end(va);
	Ui_errorText(this, this->message);
}

///+++++++++++++++++++++++++ Ui_printMenu01 ++++++++++++++++++++++++++++++
void Ui_printMenu01(
	Ui*            this,
	char*          plotFile,
	double         scale,
	double         width,
	double         height,
	uint8_t        moveLength,
	plotter_mode_t plMode
) {
	double s2 = scale / 10.0 / STEPS_PERMM_X;
	double w  = width  * s2;
	double h  = height * s2;

	Ui_clrscr(this, 1, this->msgY-2);
	Ui_msg(this, 1, 1, 1,      "*** Main menu plotter ***");
	Ui_msgArg(this, 10, 3, 0,  "M            - toggle move length, current value = %ld step(s)", moveLength);
	Ui_msg(this, 10, 4, 0,     "Cursor right - move plotter in positive X direction");
	Ui_msg(this, 10, 5, 0,     "Cursor left  - move plotter in negative X direction");
	Ui_msg(this, 10, 6, 0,     "Cursor up    - move plotter in positive Y direction");
	Ui_msg(this, 10, 7, 0,     "Cursor down  - move plotter in negative Y direction");
	Ui_msg(this, 10, 8, 0,     "Page up      - lift pen");
	Ui_msg(this, 10, 9, 0,     "Page down    - touch down pen");
	Ui_msgArg(this, 10, 10, 0, "F            - choose file. Current file = \"%s\"", plotFile);
	Ui_msgArg(this, 10, 11, 0, "               Scale set to = %0.4f. W = %0.2fcm, H = %0.2fcm", scale, w, h);
	Ui_msg(this, 10, 12, 0,    "P            - plot file");

	if (plMode == PLOTTER_MODE_PRINT) {
		Ui_msg(this, 10, 13, 0, "               Operating mode: PRINTING");
	}
	else {
		if (plMode == PLOTTER_MODE_PLOT) {
			Ui_msg(this, 10, 13, 0, "               Operating mode: PLOTTING");
		}
	}

	Ui_msg(this, 10, 16, 0, "Esc          - leave program");
}

///+++++++++++++++++++++++++ Ui_printMenu02 ++++++++++++++++++++++++++++++
void Ui_printMenu02(
	Ui*          this,
	uint16_t     startRow,
	b8           selected,
	char*        fileName,
	size_t       fileNameSize,
	const char*  picturePath
) {
	DIR           *pDIR;
	struct dirent *pDirEnt;
	uint8_t       i = 0;
	uint8_t       discard = 0;
	char          cSel[2];

	Ui_clrscr(this, 1, this->msgY-2);
	Ui_msg(this, 1, 1, 1, "*** Choose plotter file ***");

	fileName[0] = '\0';

	if (! (pDIR = opendir(picturePath))) {
		Ui_msgArg(this, 1, 4, 1, "Could not open directory '%s' [%s]!", picturePath, geterr());
		Ui_getch();
		return;
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
						snprintf(fileName, fileNameSize, "%s", pDirEnt->d_name);
					}
					else {
						cSel[0] = cSel[1] = ' ';
					}
					Ui_msgArg(this, 1, 3 + i, 0, "%c%s%c", cSel[0], pDirEnt->d_name, cSel[1]);
					i++;
				}
				discard++;
			}
		}
	}
	Ui_gotoxy(this->msgX, this->msgY + 1);
	printf("Choose file using up/down keys and confirm with 'Enter' or press 'Esc' to cancel.");
}

///+++++++++++++++++++++++++ Ui_printMenu03 ++++++++++++++++++++++++++++++
void Ui_printMenu03(
	Ui*         this,
	const char* fullFileName,
	uint32_t    numberOfLines,
	long        currentLine,
	long        currentX,
	long        currentY,
	long        startTime
) {
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

	Ui_clrscr(this, 1, this->msgY - 2);
	Ui_msg(this,    1,  1, 1, "*** Plotting file ***");
	Ui_msgArg(this, 10, 3, 0, "File name: %s", fullFileName);
	Ui_msgArg(this, 10, 4, 0, "Number of lines: %ld", numberOfLines);
	Ui_msgArg(this, 10, 5, 0, "Current Position(%ld): X = %ld, Y = %ld     ", currentLine, currentX, currentY);
	Ui_msgArg(this, 10, 6, 0, "Process time: %02u:%02u:%02u", processHours, processMinutes, processSeconds);
}

void Ui_msg(Ui *this, uint16_t x, uint16_t y, b8 alignment, const char *msg) {
	Ui_clrscr(this, y, y);
	Ui_gotoxy(x, y);

	if (alignment) {
		uint16_t i;
		y = (this->maxCols - strlen(msg)) / 2;

		for (i = 0; i < y; i++) {
			putchar(' ');
		}
	}
	puts(msg);
}

void Ui_msgArg(Ui *this, uint16_t x, uint16_t y, b8 alignment, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vsnprintf(this->message, sizeof(this->message), fmt, va);
	va_end(va);
	Ui_msg(this, x, y, alignment, this->message);
}
