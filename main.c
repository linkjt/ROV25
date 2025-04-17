#include "gpio.h"
#include "
// defined pins, change this later
#define ROVUP 1
#define ROVDOWN 2
#define ROVX 3
#define ROVY 4

#define ARMPIN1 10
#define ARMPIN2 11

typedef struct{
int rightjoyX;
int rightjoyY;
int leftjoyX;
int leftjoyY;
int lefttrigger;
int righttrigger;
bool Ybutton;
bool Xbutton;
bool Bbutton;
bool Abutton;
bool Backbutton;
bool Startbutton;
bool Modebutton;
bool Dpadup;
bool Dpadleft;
bool Dpadright;
bool Dpaddown;
} controller;

bool fastslowmode = false;


controller logi; // get this data from somewhere using the control data thing I made

gpiostart();
gpioSetMode(ROVUP,PI_OUTPUT);
gpioSetMode(ROVDOWN,PI_OUTPUT);
gpioSetMode(ROVX,PI_OUTPUT);
gpioSetMode(ROVY,PI_OUTPUT);
gpioSetMode(ARMPIN1,PI_OUTPUT);
gpioSetMode(ARMPIN2,PI_OUTPUT);
while(1=1){
//fast and slow mode
if((fastslowmode & logi.Startbutton)||(!fastslowmode & logi.Startbutton)){
fastslowmode = !fastslowmode;
logi.Startbutton = false;
}

// Maybe use a switch case, not rly sure rn, I JUST NEED TO KNOW WIREING

gpioServo(ROVUP, 1500+200*(logi.Dpadup)*(1+fastslowmode)-200*(logi.Dpaddown)*(1+fastslowmode));
gpioServo(ROVX, 1500+(logi.leftjoyX)*(1+fastslowmode));
gpioServo(ROVY, 1500+(logi.leftjoyY)*(1+fastslowmode));


//fidle around till controls are good
gpioServo(ARMPIN1, 1500+logi.rightjoyY*10*(1+fastslowmode));
//fidle around till controls are good
gpioServo(ARMPIN2, 1500+logi.rightjoyX*10*(1+fastslowmode));


