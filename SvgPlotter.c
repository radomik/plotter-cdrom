#include "SvgPlotter.h"
#include "Controller.h"
#include "Ui.h"
#include "Vars.h"

void SvgPlotter_plot(
	Controller* ctr,
	Ui*         ui,
	Vars*       vars,
	b8          draw
) {
	char    textLine[300];
	FILE*   plotFile = vars->plotFile;
	char*   pEnd = NULL;
	long    currentPlotX=0, currentPlotY=0;
	long    xNow  =  0, yNow  =  0;
	long    xNow1 = -1, yNow1 = -1;
	long    xNow2 = -1, yNow2 = -1;
	long    plotStartTime = 0;
	int     i;
	char    a;
	uint8_t rdState = 0;
	b8      stopPlot = false;
	int     ret;

	if (draw) {
		plotStartTime = time(0);

		Ui_printMenu03(ui, vars->fullFileName, vars->coordinateCount, 0, 0, 0, plotStartTime);

		Controller_penUp(ctr);
	}

	while (! (feof(plotFile) || stopPlot)) {
		if ((ret=fread(&a, 1, 1, plotFile)) != 1) {
			if (ret == 0) break; // EOF
			Ui_errorTextArg(ui, "Error reading file [ret=%d, %s]", ret, geterr());
			Controller_penUp(ctr);
			return;
		}
		i = 0;
		textLine[0] = '\0';
		while (a != ' ' && a != '<' && a != '>' && a != '\"' && a != '=' && a != ',' && a != ':') {
			textLine[i] = a;
			textLine[++i] = '\0';
			if ((ret=fread(&a, 1, 1, plotFile)) != 1) {
				Ui_errorTextArg(ui, "Error reading file (2) [ret=%d, %s]", ret, geterr());
				Controller_penUp(ctr);
				return;
			}
		}

		if (a == '<') { // Init
			if (draw) {
				if (xNow2 > -1 && yNow2 > -1 && (xNow2 != xNow1 || yNow2 != yNow1)) {
					stopPlot = Controller_calcPlotter(ctr, ui, xNow2 - currentPlotX, yNow2 - currentPlotY);
					Controller_penDown(ctr);

					currentPlotX = xNow2;
					currentPlotY = yNow2;

					stopPlot = Controller_calcPlotter(ctr, ui, xNow1 - currentPlotX, yNow1 - currentPlotY);
					stopPlot = Controller_calcPlotter(ctr, ui, xNow1 - currentPlotX, yNow1 - currentPlotY);
					currentPlotX = xNow1;
					currentPlotY = yNow1;

					stopPlot = Controller_calcPlotter(ctr, ui, xNow - currentPlotX, yNow - currentPlotY);
					currentPlotX = xNow;
					currentPlotY = yNow;
				}

				xNow1 = xNow2 = -1;
				yNow1 = yNow2 = -1;
			}
			rdState = 0;
		}
		if (! strcmp(textLine, "path")) {
			if (draw) {
				Controller_penUp(ctr);
			}
			rdState = 1; // path found
		}
		if ((rdState == 1) && (! strcmp(textLine, "fill"))) {
			rdState = 2; // fill found
		}
		if ((rdState == 2) && (! strcmp(textLine, "none"))) {
			rdState = 3; // none found
		}
		if ((rdState == 2) && (! strcmp(textLine, "stroke"))) {
			rdState = 0; // stroke found, fill isn't "none"
		}
		if ((rdState == 3) && (a == '=') && (! strcmp(textLine, "d"))) {
			rdState = 4; // d= found
		}
		if ((rdState == 4) && (a == ' ') && (! strcmp(textLine, "M"))) {
			rdState = 5; // M found
		}
		if ((! draw) && (rdState == 5) && (a == ' ') && (! strcmp(textLine, "C"))) {
			rdState = 5; // C found
		}
		if (rdState == 6) { // Y value
			if (draw) {
				yNow = (strtol(textLine, &pEnd, 10) - vars->yMin) * vars->scale * STEPS_PERMM_Y / STEPS_PERMM_X;
			}
			else {
				yNow = strtol(textLine, &pEnd, 10);
				if (yNow > vars->yMax) {
					vars->yMax = yNow;
				}
				else {
					if (yNow < vars->yMin) {
						vars->yMin = yNow;
					}
				}
			}
			rdState = 7;
		}
		if (rdState == 5 && a == ',') { // X value
			if (draw) {
				xNow = ((vars->xMax - strtol(textLine, &pEnd, 10))) * vars->scale;
			}
			else {
				xNow = strtol(textLine, &pEnd, 10);
				if (xNow > vars->xMax) {
					vars->xMax = xNow;
				}
				else {
					if (xNow < vars->xMin) {
						vars->xMin = xNow;
					}
				}
			}

			rdState = 6;
		}
		if (rdState == 7) {
			if (draw) {
				if (xNow2 > -1 && yNow2 > -1 && (xNow2 != xNow1 || yNow2 != yNow1)) {
					stopPlot = Controller_calcPlotter(ctr, ui, xNow2 - currentPlotX, yNow2 - currentPlotY);
					Controller_penDown(ctr);
					currentPlotX = xNow2;
					currentPlotY = yNow2;
				}
				xNow2 = xNow1;
				yNow2 = yNow1;
				xNow1 = xNow;
				yNow1 = yNow;
			}
			rdState = 5;
		}

		if (! draw) {
			Ui_gotoxy(1, ui->msgY);
			printf("rdState=%3d, xNow=%6ld, xMin=%6d, xMax=%6d, yMin=%6d, yMax=%6d   ",
				rdState, xNow, vars->xMin, vars->xMax, vars->yMin, vars->yMax);
		}
	}

	if (draw) {
		Controller_penUp(ctr);
		Ui_printMenu03(ui, vars->fullFileName, vars->coordinateCount, 0, 0, 0, plotStartTime);
		Controller_calcPlotter(ctr, ui, -currentPlotX, -currentPlotY);
	}
}
