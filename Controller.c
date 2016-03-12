#include "Controller.h"
#include "Gpio.h"
#include "Ui.h"

static void Controller_makeStepX(Controller* this, int direction) {
	this->stepX += direction;

	if (this->stepX > 3) {
		this->stepX = 0;
	}
	else {
		if (this->stepX < 0) {
			this->stepX = 3;
		}
	}

	// You might have to swap the sequence of steps!!!
	// If your motor doesn't rotate as expected, try:
	//  .stepX == 0   .stepX == 1   .stepX == 2   .stepX == 3
	//  1 0 0 0      0 0 0 1      0 1 0 0      0 0 1 0
	// or:
	//  .stepX == 0   .stepX == 1   .stepX == 2   .stepX == 3
	//  1 0 0 0      0 0 1 0      0 1 0 0      0 0 0 1
	// or:
	//  .stepX == 0   .stepX == 1   .stepX == 2   .stepX == 3
	//  1 0 0 0      0 1 0 0      0 0 1 0      0 0 0 1
	// or:
	//  .stepX == 0   .stepX == 1   .stepX == 2   .stepX == 3
	//  1 0 0 0      0 1 0 0      0 0 0 1      0 0 1 0

	switch (this->stepX) {
		case 0:
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER01, 1);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER04, 0);
			break;
		case 1:
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER03, 1);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER04, 0);
			break;
		case 2:
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER02, 1);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER04, 0);
			break;
		case 3:
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_X_STEPPER04, 1);
			break;
	}

	usleep(STEP_PAUSE);
}

static void Controller_makeStepY(Controller* this, int direction) {
	this->stepY += direction;

	if (this->stepY > 3) {
		this->stepY = 0;
	}
	else {
		if (this->stepY < 0) {
			this->stepY = 3;
		}
	}

	// You might have to swap the sequence of steps!!!
	// If your motor doesn't rotate as expected, try:
	//  .stepY == 0   .stepY == 1   .stepY == 2   .stepY == 3
	//  1 0 0 0      0 0 0 1      0 1 0 0      0 0 1 0
	// or:
	//  .stepY == 0   .stepY == 1   .stepY == 2   .stepY == 3
	//  1 0 0 0      0 0 1 0      0 1 0 0      0 0 0 1
	// or:
	//  .stepY == 0   .stepY == 1   .stepY == 2   .stepY == 3
	//  1 0 0 0      0 1 0 0      0 0 1 0      0 0 0 1
	// or:
	//  .stepY == 0   .stepY == 1   .stepY == 2   .stepY == 3
	//  1 0 0 0      0 1 0 0      0 0 0 1      0 0 1 0

	switch (this->stepY) {
		case 0:
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER01, 1);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER04, 0);
			break;
		case 1:
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER03, 1);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER04, 0);
			break;
		case 2:
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER02, 1);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER04, 0);
			break;
		case 3:
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_Y_STEPPER04, 1);
			break;
	}

	usleep(STEP_PAUSE);
}

void Controller_makeStepZ(Controller* this, int direction) {
	this->stepZ += direction;

	if (this->stepZ > 3) {
		this->stepZ = 0;
	}
	else {
		if (this->stepZ < 0) {
			this->stepZ = 3;
		}
	}

	// You might have to swap the sequence of steps!!!
	// If your motor doesn't rotate as expected, try:
	//  .stepZ == 0   .stepZ == 1   .stepZ == 2   .stepZ == 3
	//  1 0 0 0      0 0 0 1      0 1 0 0      0 0 1 0
	// or:
	//  .stepZ == 0   .stepZ == 1   .stepZ == 2   .stepZ == 3
	//  1 0 0 0      0 0 1 0      0 1 0 0      0 0 0 1
	// or:
	//  .stepZ == 0   .stepZ == 1   .stepZ == 2   .stepZ == 3
	//  1 0 0 0      0 1 0 0      0 0 1 0      0 0 0 1
	// or:
	//  .stepZ == 0   .stepZ == 1   .stepZ == 2   .stepZ == 3
	//  1 0 0 0      0 1 0 0      0 0 0 1      0 0 1 0

	switch (this->stepZ) {
		case 0:
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER01, 1);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER04, 0);
			break;
		case 1:
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER03, 1);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER04, 0);
			break;
		case 2:
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER02, 1);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER04, 0);
			break;
		case 3:
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER01, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER02, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER03, 0);
			Gpio_digitalWrite(GPIO_PIN_Z_STEPPER04, 1);
			break;
	}

	usleep(STEP_PAUSE);
}

void Controller_new(Controller* this) {
	this->stepX = 0;
	this->stepY = 0;
	this->stepZ = 0;
	this->currentPlotDown = false;
	this->moveLength = 1;
}

void Controller_toggleMoveLength(Controller* this) {
	if (this->moveLength == 1) {
		this->moveLength = 10;
	}
	else {
		this->moveLength = 1;
	}
}

void Controller_initOrExit(Controller* this) {
	Gpio_initOrExit();
	Controller_forceMovePen(this, false);
}

void Controller_cleanUpAndExit() {
	Gpio_cleanupAndExit();
}

void Controller_forceMovePen(Controller* this, b8 down) {
	Gpio_softPwmWrite(GPIO_PIN_Z_SERVO, down ? GPIO_SERVODOWN : GPIO_SERVOUP);
	usleep(SERVO_PAUSE);
	Gpio_softPwmWrite(GPIO_PIN_Z_SERVO, 0);
	this->currentPlotDown = down;
}

void Controller_penDown(Controller* this) {
	if (this->currentPlotDown) Controller_forceMovePen(this, true);
}

void Controller_penUp(Controller* this) {
	if (! this->currentPlotDown) Controller_forceMovePen(this, false);
}

/// @return value indicating whether plot should be stopped
///         for now it always return false
b8 Controller_calcPlotter(
	Controller* this,
	Ui*         ui,
	int32_t     moveX,
	int32_t     moveY
) {
	long tempX = 0, tempY = 0;
	int  i = 0;

	Ui_msgArg(ui, ui->msgX, ui->msgY, 0, "Moving X: %ld, Moving Y: %ld", moveX, moveY);

	if (moveX == 0) {
		if (moveY > 0) {
			for (i = 0; i < moveY; i++) {
				Controller_makeStepY(this, -1);
			}
		}
		else {
			if (moveY < 0) {
				for (i = 0; i < -moveY; i++) {
					Controller_makeStepY(this, 1);
				}
			}
		}
	}

	if (moveY == 0) {
		if (moveX > 0) {
			for (i = 0; i < moveX; i++) {
				Controller_makeStepX(this, 1);
			}
		}
		else {
			if (moveX < 0) {
				for (i = 0; i < -moveX; i++) {
					Controller_makeStepX(this, -1);
				}
			}
		}
	}

	if (moveY != 0 && moveX != 0) {
		if (abs(moveX) > abs(moveY)) {
			while (moveY != 0) {
				tempX = moveX / abs(moveY);
				if (tempX == 0) {
					printf("tempX=%ld, moveX=%d, moveY=%d    \n", tempX, moveX, moveY);
				}
				else {
					if (tempX > 0) {
						for (i = 0; i < tempX; i++) {
							Controller_makeStepX(this, 1);
						}
					}
					else { // tempX < 0
						for (i = 0; i < -tempX; i++) {
							Controller_makeStepX(this, -1);
						}
					}
				}
				moveX -= tempX;
				if (moveY > 0) {
					Controller_makeStepY(this, -1);
					moveY--;
				}
				else {
					if (moveY < 0) {
						Controller_makeStepY(this, 1);
						moveY++;
					}
				}
			} // while (moveY != 0)

			// move remaining X coordinates
			if (moveX > 0) {
				for (i = 0; i < moveX; i++) {
					Controller_makeStepX(this, 1);
				}
			}
			else {
				if (moveX < 0) {
					for (i = 0; i < -moveX; i++) {
						Controller_makeStepX(this, -1);
					}
				}
			}
		}
		else { // abs(moveX) <= abs(moveY)
			while (moveX != 0) {
				tempY = moveY / abs(moveX);
				if (tempY == 0) {
					printf("tempY=%ld, moveX=%d, moveY=%d    \n", tempX, moveX, moveY);
				}
				else {
					if (tempY > 0) {
						for (i = 0; i < tempY; i++) {
							Controller_makeStepY(this, -1);
						}
					}
					else { // tempY < 0
						for (i = 0; i < -tempY; i++) {
							Controller_makeStepY(this, 1);
						}
					}
				}
				moveY -= tempY;
				if (moveX > 0) {
					Controller_makeStepX(this, 1);
					moveX--;
				}
				else {
					if (moveX < 0) {
						Controller_makeStepX(this, -1);
						moveX++;
					}
				}
			}

			// move remaining Y coordinates
			if (moveY > 0) {
				for (i = 0; i < moveY; i++) {
					Controller_makeStepY(this, -1);
				}
			}
			else {
				if (moveY < 0) {
					for (i = 0; i < -moveY; i++) {
						Controller_makeStepY(this, 1);
					}
				}
			}
		}
	}
	return false;
}
