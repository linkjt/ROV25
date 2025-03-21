#include "gpio.h"

gpiostart();
gpioSetMode(17,PI_OUTPUT);
gpioWrite(17, 1);
