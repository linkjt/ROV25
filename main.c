#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>
#include <linux/joystick.h>

#define UART_DEVICE "/dev/ttyS0"
#define BAUD_RATE B115200
#define MAX_BUFFER_SIZE 512

#define ROVUP1 17
#define ROVUP2 27
#define ROVM1 5
#define ROVM2 6
#define ROVM3 19
#define ROVM4 26

#define ARMPIN1 9
#define ARMPIN2 10

#define HANDCODE 11

typedef struct {
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
    bool Dpadup;
    bool Dpadleft;
    bool Dpadright;
    bool Dpaddown;
} controller;

// Scale joystick axis input to a range of -500 to +500
int scale_axis(int value) {
    return (int)(value * 500.0 / 32767.0);
}

// Clamp a value between 500 and 2500 (for servo)
int clamp_servo(int value) {
    if (value < 500) return 500;
    if (value > 2500) return 2500;
    return value;
}

int update_logitech_state(int fd, controller *state) {
    struct js_event js;
    ssize_t bytes_read;

    bytes_read = read(fd, &js, sizeof(js));

    if (bytes_read == sizeof(js)) {
        if (js.type & JS_EVENT_AXIS) {
            switch (js.number) {
                case 0: state->leftjoyX = js.value; break;
                case 1: state->leftjoyY = -js.value; break;
                case 2: state->rightjoyX = js.value; break;
                case 3: state->rightjoyY = -js.value; break;
                case 4: state->lefttrigger = js.value; break;
                case 5: state->righttrigger = js.value; break;
                case 6: // D-pad Left/Right
                    state->Dpadleft = (js.value < 0);
                    state->Dpadright = (js.value > 0);
                    if (js.value == 0) {
                        state->Dpadleft = false;
                        state->Dpadright = false;
                    }
                    break;
                case 7: // D-pad Up/Down
                    state->Dpadup = (js.value < 0);
                    state->Dpaddown = (js.value > 0);
                    if (js.value == 0) {
                        state->Dpadup = false;
                        state->Dpaddown = false;
                    }
                    break;
            }
        } else if (js.type & JS_EVENT_BUTTON) {
            switch (js.number) {
                case 3: state->Ybutton = js.value; break;
                case 2: state->Xbutton = js.value; break;
                case 1: state->Bbutton = js.value; break;
                case 0: state->Abutton = js.value; break;
                case 6: state->Backbutton = js.value; break;
                case 7: state->Startbutton = js.value; break;
            }
        }
        return 1;
    } else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;
    } else {
        perror("Error reading joystick event");
        return -1;
    }
}

void servomove(controller logi) {
    int lx = scale_axis(logi.leftjoyX);
    int ly = scale_axis(logi.leftjoyY);
    int rx = scale_axis(logi.rightjoyX);
    int ry = scale_axis(logi.rightjoyY);

    int lt = scale_axis(logi.lefttrigger);
    int rt = scale_axis(logi.righttrigger);

    int boost = (1 + logi.Startbutton);

    gpioServo(ROVUP1, clamp_servo(1500 + 100 * logi.Dpadup * boost - 100 * logi.Dpaddown * boost + 50 * logi.Bbutton - 50 * logi.Xbutton));
    gpioServo(ROVUP2, clamp_servo(1500 + 100 * logi.Dpadup * boost - 100 * logi.Dpaddown * boost + 50 * logi.Xbutton - 50 * logi.Bbutton));

    gpioServo(ROVM1, clamp_servo(1500 + lx * boost + ly * boost + 50 * logi.Ybutton - 50 * logi.Abutton));
    gpioServo(ROVM2, clamp_servo(1500 + lx * boost - ly * boost + 50 * logi.Ybutton - 50 * logi.Abutton));
    gpioServo(ROVM3, clamp_servo(1500 - lx * boost + ly * boost + 50 * logi.Ybutton - 50 * logi.Abutton));
    gpioServo(ROVM4, clamp_servo(1500 - lx * boost - ly * boost + 50 * logi.Ybutton - 50 * logi.Abutton));

    gpioServo(HANDCODE, clamp_servo(1500 + lt * boost - rt * boost));

    gpioServo(ARMPIN1, clamp_servo(1500 + ry * boost));
    gpioServo(ARMPIN2, clamp_servo(1500 + rx * boost));
}

int main() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize GPIO\n");
        return 1;
    }

    int fd;
    controller logi_state = {0};
    controller logi;

    const char *device_path = "/dev/input/js0";
    fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("Error opening joystick");
        return 1;
    }

    gpioSetMode(ROVUP1, PI_OUTPUT);
    gpioSetMode(ROVUP2, PI_OUTPUT);
    gpioSetMode(ROVM1, PI_OUTPUT);
    gpioSetMode(ROVM2, PI_OUTPUT);
    gpioSetMode(ROVM3, PI_OUTPUT);
    gpioSetMode(ROVM4, PI_OUTPUT);
    gpioSetMode(ARMPIN1, PI_OUTPUT);
    gpioSetMode(ARMPIN2, PI_OUTPUT);
    gpioSetMode(HANDCODE, PI_OUTPUT);

    while (1) {
        memset(&logi_state, 0, sizeof(controller));

        if (update_logitech_state(fd, &logi_state) > 0) {
            logi = logi_state;
            servomove(logi);
        }
        usleep(50000); // 50ms
    }

    close(fd);
    return 0;
}
