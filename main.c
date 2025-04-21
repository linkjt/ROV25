#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define UART_DEVICE "/dev/ttyS0"
#define BAUD_RATE B115200       // Must match the Arduino's baud rate
// defined pins, change this later

#define ROVUP1 1
#define ROVUP2 1
#define ROVM1 2
#define ROVM2 3
#define ROVM3 4
#define ROVM4 5

#define ARMPIN1 10
#define ARMPIN2 11

#define HANDCODE 12

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



controller logi; // get this data from somewhere using the control data thing I made

gpiostart();
gpioSetMode(ROVUP1,PI_OUTPUT);
gpioSetMode(ROVUP2,PI_OUTPUT);

gpioSetMode(ROVM1,PI_OUTPUT);
gpioSetMode(ROVM2,PI_OUTPUT);
gpioSetMode(ROVM3,PI_OUTPUT);
gpioSetMode(ROVM4,PI_OUTPUT);


gpioSetMode(ARMPIN1,PI_OUTPUT);
gpioSetMode(ARMPIN2,PI_OUTPUT);

gpioSetMode(HANDCODE,PI_OUTPUT);
bool servomove(controllor logi);

int main() {
    int uart_fd;
    struct termios options;
    char buffer[256];
    int bytes_read;

    // Open the UART device
    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        perror("Error opening UART");
        return -1;
    }

    // Get the current terminal settings
    if (tcgetattr(uart_fd, &options) < 0) {
        perror("Error getting termios settings");
        close(uart_fd);
        return -1;
    }

    // Set the baud rate, make sure ts same as arduino
    options.c_cflag = BAUD_RATE | CS8 | CLOCAL | CREAD;
    options.c_iflag = 0;
    options.c_oflag = 0;
    options.c_lflag = 0;

    // Apply the new settings
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &options) < 0) {
        perror("Error setting termios settings");
        close(uart_fd);
        return -1;
    }

    printf("Listening for data on %s at %d baud...\n", UART_DEVICE, BAUD_RATE);

    int buffer_index =0;
    while (1) {
        // read data from arduino
        bytes_read = read(uart_fd, buffer + buffer_index, sizeof(buffer) - 1 - buffer_index);
        //ts is if we actually read stuff
        if (bytes_read >= 0) {
            buffer_index += bytes_read;
            buffer[buffer_index] = '\0' //set up so non NULL

            char *newline_pos = strchr(buffer,'\n');
        ////// ADD HERE
        //error handeling
        } else if (bytes_read < 0) {
            perror("Error reading from UART");
            break;
        }
        usleep(100); // Small delay to avoid busy-waiting
    }

    close(uart_fd);
    return 0;
}

bool servomove(controller logi, bool fastmode){
//fast and slow mode
if((fastslowmode & logi.Startbutton)||(!fastslowmode & logi.Startbutton)){
fastslowmode = !fastslowmode;
logi.Startbutton = false;
}

// Maybe use a switch case, not rly sure rn, I JUST NEED TO KNOW WIREING

gpioServo(ROVUP1, 1500+200*(logi.Dpadup)*(1+fastslowmode)-200*(logi.Dpaddown)*(1+fastslowmode));
gpioServo(ROVUP2, 1500+200*(logi.Dpadup)*(1+fastslowmode)-200*(logi.Dpaddown)*(1+fastslowmode));
//X movement
gpioServo(ROVM1, 1500+(logi.leftjoyX)*(1+fastslowmode));
gpioServo(ROVM2, 1500+(logi.leftjoyX)*(1+fastslowmode));
gpioServo(ROVM3, 1500-(logi.leftjoyX)*(1+fastslowmode));
gpioServo(ROVM4, 1500-(logi.leftjoyX)*(1+fastslowmode));

gpioServo(ROVM1, 1500+(logi.leftjoyY)*(1+fastslowmode));
gpioServo(ROVM3, 1500+(logi.leftjoyY)*(1+fastslowmode));
gpioServo(ROVM2, 1500-(logi.leftjoyY)*(1+fastslowmode));
gpioServo(ROVM4, 1500-(logi.leftjoyY)*(1+fastslowmode));

gpioServo(HANDCODE, 1500+200*(logi.lefttrigger)*(1+fastslowmode)-200*(logi.righttrigger)*(1+fastslowmode));
//fidle around till controls are good
gpioServo(ARMPIN1, 1500+logi.rightjoyY*10*(1+fastslowmode));
//fidle around till controls are good
gpioServo(ARMPIN2, 1500+logi.rightjoyX*10*(1+fastslowmode));
}


