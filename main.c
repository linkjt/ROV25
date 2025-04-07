#include "gpio.h"
#include "
// defined pins, change this later
#define ROVUP 1
#define ROVDOWN 2
#define ROVX 3
#define ROVY 4
controller logi; // get this data from somewhere using the control data thing I made

gpiostart();
gpioSetMode(ROVUP,PI_OUTPUT);
gpioSetMode(ROVDOWN,PI_OUTPUT);
gpioSetMode(ROVX,PI_OUTPUT);
gpioSetMode(ROVY,PI_OUTPUT);
while(1=1){


gpioServo(ROVX, 1500);
gpioServo(ROVY, 1500);
if(logi.Dpadup){
  gpioServo(ROVUP, 1700);
} else{
gpioServo(ROVUP, 1500);
}
if(logi.Dpaddown){
  gpioServo(ROVDOWN, 1700);
}else{
  gpioServo(ROVDOWN, 1500);
}
if(logi.leftjoyX!=0){
  gpioServo(ROVX, 1500+logi.leftjoyX);
}else{
  gpioServo(ROVX, 1500);
}
if(logi.leftjoyY!=0){
  gpioServo(ROVY, 1500+logi.leftjoyY);
}else{
  gpioServo(ROVY, 1500);
}


