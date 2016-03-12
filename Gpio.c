#include "Gpio.h"

void Gpio_initOrExit() {
#ifndef GPIO_MOCK
	if (wiringPiSetup() == -1) {
		fprintf(stderr, "Could not run wiringPiSetup! [%s]\n", geterr());
		exit(1);
	}

	softPwmCreate(GPIO_PIN_Z_SERVO, GPIO_SERVOUP, 200);

	pinMode(GPIO_PIN_X_STEPPER01, OUTPUT);
	pinMode(GPIO_PIN_X_STEPPER02, OUTPUT);
	pinMode(GPIO_PIN_X_STEPPER03, OUTPUT);
	pinMode(GPIO_PIN_X_STEPPER04, OUTPUT);

	Gpio_digitalWrite(GPIO_PIN_X_STEPPER01, 1);
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER02, 0);
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER03, 0);
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER04, 0);

	pinMode(GPIO_PIN_Y_STEPPER01, OUTPUT);
	pinMode(GPIO_PIN_Y_STEPPER02, OUTPUT);
	pinMode(GPIO_PIN_Y_STEPPER03, OUTPUT);
	pinMode(GPIO_PIN_Y_STEPPER04, OUTPUT);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER01, 1);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER02, 0);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER03, 0);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER04, 0);
#endif
}

void Gpio_cleanupAndExit() {
#ifndef GPIO_MOCK
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER01, 0);
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER02, 0);
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER03, 0);
	Gpio_digitalWrite(GPIO_PIN_X_STEPPER04, 0);

	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER01, 0);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER02, 0);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER03, 0);
	Gpio_digitalWrite(GPIO_PIN_Y_STEPPER04, 0);
#endif
	exit(0);
}
