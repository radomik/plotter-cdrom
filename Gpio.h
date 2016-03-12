#ifndef PLOTTER_CDROM_GPIO_H
#define PLOTTER_CDROM_GPIO_H

#include "common.h"

#define GPIO_SERVOUP     10
#define GPIO_SERVODOWN   20

#define GPIO_PIN_X_STEPPER01 2
#define GPIO_PIN_X_STEPPER02 0
#define GPIO_PIN_X_STEPPER03 4
#define GPIO_PIN_X_STEPPER04 1

#define GPIO_PIN_Y_STEPPER01 13
#define GPIO_PIN_Y_STEPPER02 14
#define GPIO_PIN_Y_STEPPER03 3
#define GPIO_PIN_Y_STEPPER04 12

#define GPIO_PIN_Z_STEPPER01 6
#define GPIO_PIN_Z_STEPPER02 5
#define GPIO_PIN_Z_STEPPER03 11
#define GPIO_PIN_Z_STEPPER04 10

#define GPIO_PIN_Z_SERVO     7

#ifndef GPIO_MOCK
	#include <wiringPi.h>
	#define Gpio_digitalWrite(pinNumber, pinState) digitalWrite(pinNumber, pinState)
	#define Gpio_softPwmWrite(pinNumber, pinState) softPwmWrite(pinNumber, pinState)
#else
	#define Gpio_digitalWrite(pinNumber, pinState) { }
	#define Gpio_softPwmWrite(pinNumber, pinState) { }
#endif

void Gpio_initOrExit();
void Gpio_cleanupAndExit();

#endif
