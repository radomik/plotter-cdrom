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

#include "common.h"
#include "Bitmap.h"
#include "Controller.h"
#include "Ui.h"
#include "Vars.h"
#include "SvgPlotter.h"
#include "BmpPlotter.h"

static char* PICTURE_PATH = NULL;

static void exiting() {
	Log_close();
	if (PICTURE_PATH) {
		free(PICTURE_PATH);
		PICTURE_PATH = NULL;
	}
}

static void setNoSelectedFile(char *fileName, size_t fileNameSize) {
	snprintf(fileName, fileNameSize, "noFiLE");
}

int main(int argc, char **argv) {
	Bitmap bmp;
	Controller ctr;
	Ui ui;
	Vars vars;
	menu_level_t menuLevel = MENU_LEVEL_MAIN;
	int keyHit = 0;
	int keyCode[5];
	char fileName[256] = "";
	char fileNameOld[256] = "";
	plotter_mode_t plMode = PLOTTER_MODE_PRINT;
	int i;
	unsigned char singleKey=0;
	int fileSelected = 0;
	uint16_t fileStartRow = 0;
	char textLine[300];

	atexit(exiting);
	Log_open();

	Controller_new(&ctr);
	Ui_new(&ui);
	Vars_new(&vars);

	if (argc > 1) {
		if (! (strcmp(argv[1], "--help"))) {
			printf("Usage:\n\
	%s [PICTURES_PATH]\n\n\
	Where PICTURES_PATH is optional argument with path to directory \n\
	containing SVG and/or BMP files. Default is: ./pictures\n", argv[0]);
			exit(0);
		}
		PICTURE_PATH = strdup(argv[1]);
	}
	else {
		char* currDir = getcwd(NULL, 0);
		if (! currDir) {
			fprintf(stderr, "Failed to get current wrking directory [%s]\n", geterr());
			exit(1);
		}

		PICTURE_PATH = calloc(strlen(currDir)+strlen(DEFAULT_PICTURE_PATH)+1, 1);
		strcat(PICTURE_PATH, currDir);
		free(currDir);

		strcat(PICTURE_PATH, DEFAULT_PICTURE_PATH);
	}

	printf("PICTURE_PATH=>%s<", PICTURE_PATH);
	l_info("Picture path: '%s'", PICTURE_PATH);

	Ui_initOrExit(&ui);
	Controller_initOrExit(&ctr);

	setNoSelectedFile(fileName, sizeof(fileName));

	Ui_clrscr(&ui, 1, ui.maxRows);
	Ui_printRow(&ui, '-', ui.msgY - 1);
	Ui_printMenu01(&ui, fileName, vars.scale, vars.xMax - vars.xMin, vars.yMax - vars.yMin, ctr.moveLength, plMode);

	while (1) {
		Ui_msg(&ui, ui.msgX, ui.msgY, 0, "Waiting for key press...");

		i = 0;
		singleKey = 1;
		memset(keyCode, 0, sizeof(keyCode));
		keyHit = 0;

		while (Ui_kbhit()) {
			keyHit = Ui_getch();
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

		if (menuLevel == MENU_LEVEL_MAIN) {
			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[4] == 0) {
				switch (keyCode[3]) {
					case 0:
						switch (keyCode[2]) {
							case 65: // move Y-axis
								Controller_calcPlotter(&ctr, &ui, 0, ctr.moveLength);
								break;
							case 66: // move Y-axis
								Controller_calcPlotter(&ctr, &ui, 0, -ctr.moveLength);
								break;
							case 67: // move X-axis
								Controller_calcPlotter(&ctr, &ui, ctr.moveLength, 0);
								break;
							case 68: // move X-axis
								Controller_calcPlotter(&ctr, &ui, -ctr.moveLength, 0);
								break;
						}
						break;
					case 126:
						switch (keyCode[2]) {
							case 53: // move pen up
								Controller_forceMovePen(&ctr, false);
								break;
							case 54: // move pen down
								Controller_forceMovePen(&ctr, true);
								break;
						}
						break;
				}
			}

			if (keyHit == ',' || keyHit == '<') {
				Controller_makeStepZ(&ctr, -ctr.moveLength);
			}

			if (keyHit == '.' || keyHit == '>') {
				Controller_makeStepZ(&ctr, ctr.moveLength);
			}

			if (keyHit == 'm') {
				Controller_toggleMoveLength(&ctr);
				Ui_printMenu01(&ui, fileName, vars.scale, vars.xMax - vars.xMin, vars.yMax - vars.yMin, ctr.moveLength, plMode);
			}

			if (keyHit == 'f') {
				fileStartRow = 0;
				fileSelected = 0;
				snprintf(fileNameOld, sizeof(fileNameOld), "%s", fileName);
				Ui_printMenu02(&ui, fileStartRow, false, fileName, sizeof(fileName), PICTURE_PATH);
				menuLevel = MENU_LEVEL_FILE_CHOOSER;
			}

			if (keyHit == 'p') { // Plot file
				Ui_msg(&ui, 1, 20, 0, "3 seconds until plotting starts !!!!!!!!!!!!!!!!!");
				sleep(3);

				if (strcmp(fileName, "noFiLE")) {
					if (Vars_openPlotFile(&vars, fileName, vars.fullFileName, textLine, sizeof(textLine)) != 0) {
						Ui_errorText(&ui, textLine);
						setNoSelectedFile(fileName, sizeof(fileName));
					}
					else { // opened file
						switch (plMode) {
							case PLOTTER_MODE_PLOT: // Plot SVG file
								SvgPlotter_plot(&ctr, &ui, &vars, true);
								Vars_closePlotFile(&vars);

								while (Ui_kbhit()) {
									Ui_getch();
								}
								Ui_msg(&ui, ui.msgX, ui.msgY, 0, "Finished! Press any key to return to main menu.");
								Ui_getch();
								Ui_printMenu01(&ui, fileName, vars.scale,
									vars.xMax - vars.xMin, vars.yMax - vars.yMin, ctr.moveLength, plMode);
								break;
							case PLOTTER_MODE_PRINT: // Print bitmap
								if (Bitmap_readHeader(&bmp, vars.plotFile, textLine, sizeof(textLine)) != 0) {
									Ui_errorTextArg(&ui, "Error reading bitmap header of file '%s' [%s]", vars.fullFileName, textLine);
								}
								else {
									BmpPlotter_plot(&bmp, &ctr, &vars, &ui);
									Vars_closePlotFile(&vars);
								}
								break;
						}
					}
				}
			} // if (keyHit == 'p')
		} // if (menuLevel == MENU_LEVEL_MAIN)

		if (menuLevel == MENU_LEVEL_FILE_CHOOSER) { // Select file

			if (keyCode[0] == 27 && keyCode[1] == 91 && keyCode[3] == 0 && keyCode[4] == 0) {
				b8 showMenu = false;
				switch (keyCode[2]) {
					case 65:
						if (fileSelected > 0) {
							fileSelected--;
							showMenu = true;
						}
						break;
					case 66:
						fileSelected++;
						showMenu = true;
						break;
				}

				if (showMenu) {
					Ui_printMenu02(&ui, fileStartRow, fileSelected, fileName, sizeof(fileName), PICTURE_PATH);
				}
			}

			if (keyHit == 10) { // Read file and store values
				menuLevel = MENU_LEVEL_MAIN;
				Ui_clrscr(&ui, ui.msgY + 1, ui.msgY + 1);
				snprintf(vars.fullFileName, sizeof(vars.fullFileName), "%s/%s", PICTURE_PATH, fileName);
				if (Vars_openPlotFile(&vars, fileName, vars.fullFileName, textLine, sizeof(textLine)) != 0) {
					Ui_errorText(&ui, textLine);
					setNoSelectedFile(fileName, sizeof(fileName));
				}
				else {
					if (memcmp(fileName + strlen(fileName)-4, ".svg", 4) == 0) {
						plMode = PLOTTER_MODE_PLOT;
					}
					else {
						plMode = PLOTTER_MODE_PRINT;
					}
					Vars_initXyMinMax(&vars);
					vars.coordinateCount = 0;

					switch (plMode) {
						case PLOTTER_MODE_PLOT:
							SvgPlotter_plot(&ctr, &ui, &vars, false);
							Vars_closePlotFile(&vars);

							if (vars.xMax - vars.xMin > vars.yMax -vars.yMin) {
								vars.scale = STEP_MAX_X / (double)(vars.xMax - vars.xMin);
							}
							else {
								vars.scale = STEP_MAX_X / (double)(vars.yMax - vars.yMin);
							}
							break;
						case PLOTTER_MODE_PRINT: // Print bitmap
							if ((Bitmap_readHeader(&bmp, vars.plotFile, textLine, sizeof(textLine)) != 0) ||
								(Bitmap_validateHeader(&bmp, textLine, sizeof(textLine)) != 0))
							{
								Ui_errorText(&ui, textLine);
								setNoSelectedFile(fileName, sizeof(fileName));
							}

							vars.xMin = vars.yMin = 0;
							vars.xMax = bmp.pictureWidth * BMP_PLOTTER_STEPS_PER_PIXEL_X;
							vars.yMax = bmp.pictureHeight * BMP_PLOTTER_STEPS_PER_PIXEL_Y;
							vars.coordinateCount = bmp.pictureWidth * bmp.pictureHeight;
							vars.scale = 1.0;
							break;
					}
				}

				Ui_printMenu01(&ui, fileName, vars.scale, vars.xMax - vars.xMin, vars.yMax - vars.yMin, ctr.moveLength, plMode);
			} // if (keyHit == 10)

		} // if (menuLevel == MENU_LEVEL_FILE_CHOOSER)

		if (keyHit == 27) {
			if (menuLevel == MENU_LEVEL_MAIN) {
				Ui_clrscr(&ui, ui.msgY + 1, ui.msgY + 1);
				Ui_msg(&ui, ui.msgX, ui.msgY + 1, 0, "Exit program (y/n)?");
				while (keyHit != 'y' && keyHit != 'n') {
					keyHit = Ui_getch();
					if (keyHit == 'y' || keyHit == 'Y') {
						Controller_cleanUpAndExit();
					}
				}
			}

			if (menuLevel == MENU_LEVEL_FILE_CHOOSER) {
				menuLevel = MENU_LEVEL_MAIN;
				snprintf(fileName, sizeof(fileName), "%s", fileNameOld);
				Ui_printMenu01(&ui, fileName, vars.scale, vars.xMax - vars.xMin, vars.yMax - vars.yMin, ctr.moveLength, plMode);
			}
			Ui_clrscr(&ui, ui.msgY + 1, ui.msgY + 1);
		}
	}

	return 0;
}
