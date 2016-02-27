#include "BmpPlotter.h"
#include "Bitmap.h"
#include "Controller.h"
#include "Vars.h"
#include "Ui.h"

const long BMP_PLOTTER_STEPS_PER_PIXEL_X = (long)( (double)STEP_MAX_X / (double)BITMAP_MAX_WIDTH  );
const long BMP_PLOTTER_STEPS_PER_PIXEL_Y = (long)( (double)STEP_MAX_Y / (double)BITMAP_MAX_HEIGHT );

void BmpPlotter_plot(
	Bitmap*      bmp,
	Controller*  ctr,
	Vars*        vars,
	Ui*          ui
) {
	FILE*         plotFile    = vars->plotFile;
	unsigned long fillBytes   = 0;
	long          currentPlotX=0, currentPlotY=0;
	b8            reverseMode = false;
	b8            newLine;
	uint8_t       bgr[3], bgrNext[3];

	while ((bmp->pictureWidth * 3 + fillBytes) % 4 != 0) {
		fillBytes++;
	}
	Controller_calcPlotter(ctr, ui, 0, BMP_PLOTTER_STEPS_PER_PIXEL_X * bmp->pictureWidth);
	fseek(plotFile, bmp->dataOffset, SEEK_SET);

	for (currentPlotY = 0; currentPlotY < bmp->pictureHeight; currentPlotY++) {
		newLine = false;
		currentPlotX = (reverseMode) ? (bmp->pictureWidth - 1) : 0;

		while (! newLine) {
			fseek(plotFile, bmp->dataOffset + (currentPlotX * bmp->pictureWidth + currentPlotX) * 3 + fillBytes * currentPlotY, SEEK_SET);
			if (fread(&bgr, sizeof(bgr), 1, plotFile) != 1) {
				Ui_errorTextArg(ui, "Error reading file [%s]", geterr());
				return;
			}

			if (reverseMode) {
				fseek(plotFile, bmp->dataOffset + (currentPlotX * bmp->pictureWidth + currentPlotX - 1) * 3 + fillBytes * currentPlotY, SEEK_SET);
			}

			if (fread(&bgrNext, sizeof(bgrNext), 1, plotFile) != 1) {
				Ui_errorTextArg(ui, "Error reading file (2) [%s]", geterr());
				return;
			}

			if (bgr[2] < 200 || bgr[1] < 200 || bgr[0] < 200) {
				Controller_penDown(ctr);

				if (bgrNext[2] > 199 && bgrNext[1] > 199 && bgrNext[0] > 199) {
					Controller_penUp(ctr);
				}
			}
			else {
				Controller_penUp(ctr);
			}

			if (! reverseMode) {
				currentPlotX++;
				if (currentPlotX < bmp->pictureWidth) {
					Controller_calcPlotter(ctr, ui, BMP_PLOTTER_STEPS_PER_PIXEL_X, 0); // X-Y movement swapped!!!
				}
				else {
					newLine = true;
					reverseMode = true;
				}
			}
			else {
				currentPlotX--;
				if (currentPlotX > -1) {
					Controller_calcPlotter(ctr, ui, -BMP_PLOTTER_STEPS_PER_PIXEL_X, 0); // X-Y movement swapped!!!
				}
				else {
					newLine = true;
					reverseMode = false;
				}
			}
		} // while (! newLine)

		Controller_penUp(ctr);
		Controller_calcPlotter(ctr, ui, 0, -BMP_PLOTTER_STEPS_PER_PIXEL_Y);
	} // for (currentPlotY = 0; currentPlotY < bmp->pictureHeight; currentPlotY++)

	Vars_closePlotFile(vars);
	Controller_penUp(ctr);

	if (reverseMode) {
		Controller_calcPlotter(ctr, ui, -BMP_PLOTTER_STEPS_PER_PIXEL_X * bmp->pictureWidth, 0);
	}
}
