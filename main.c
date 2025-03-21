#include "gpio.h"

gpiostart();
gpioSetMode(17,PI_OUTPUT);
gpioServo(17, 1500);
gpioServo(17,1900);
