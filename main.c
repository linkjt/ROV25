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


#define UART_DEVICE "/dev/ttyS0" // if Uart not working, switch to AMA0, i dunno
#define BAUD_RATE B115200
#define MAX_BUFFER_SIZE 512 // Increased buffer size to accommodate more data

#define ROVUP1 0
#define ROVUP2 2
#define ROVM1 24
#define ROVM2 25
#define ROVM3 21
#define ROVM4 22

#define ARMPIN1 10
#define ARMPIN2 11

#define HANDCODE 9

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
} controller; //controller structure used

// Function to read and update the controller state, just get based on the type of event and event number
int update_logitech_state(int fd, controller *state) {
    struct js_event js;
    ssize_t bytes_read;

    bytes_read = read(fd, &js, sizeof(js));

    if (bytes_read == sizeof(js)) {
        // type is event axis
        if (js.type & JS_EVENT_AXIS) {
            switch (js.number) {
                case 0: state->leftjoyX = js.value; break;   
                case 1: state->leftjoyY = -js.value; break;
                case 2: state->rightjoyX = js.value; break;
                case 3: state->rightjoyY = -js.value; break;
                case 4: state->lefttrigger = js.value; break;
                case 5: state->righttrigger = js.value; break;
            }
        } else if (js.type & JS_EVENT_BUTTON) {
            switch (js.number) {
                // button values, have type event button 
                case 3: state->Ybutton = js.value; break;    
                case 2: state->Xbutton = js.value; break;   
                case 1: state->Bbutton = js.value; break;     
                case 0: state->Abutton = js.value; break;     
                case 6: state->Backbutton = js.value; break; 
                case 7: state->Startbutton = js.value; break; 
            }
            //Dpad values, all on number
        } else if (js.type & JS_EVENT_HAT) {
            switch (js.number) {
                case 0:
                    state->Dpadup = (js.value == -1 || js.value == -3); 
                    state->Dpaddown = (js.value == 1 || js.value == 3);    
                    state->Dpadleft = (js.value == -1 || js.value == -2);
                    state->Dpadright = (js.value == 1 || js.value == 2);
                    break;
            }
        }
        return 1;
        //error handeling
    } else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;
    } else {
        perror("Error reading joystick event");
        return -1;
    }
}


void servomove(controller logi){

// Maybe use a switch case, not rly sure rn, I JUST NEED TO KNOW WIREING

gpioServo(ROVUP1, 1500+200*(logi.Dpadup)*(1+logi.Startbutton)-200*(logi.Dpaddown)*(1+logi.Startbutton)+50*(logi.Bbutton)-50*(logi.Xbutton));
gpioServo(ROVUP2, 1500+200*(logi.Dpadup)*(1+logi.Startbutton)-200*(logi.Dpaddown)*(1+logi.Startbutton)+50*(logi.Xbutton)-50*(logi.Bbutton));
//X movement
gpioServo(ROVM1, 1500+100*(logi.leftjoyX)*(1+logi.Startbutton)+100*(logi.leftjoyY)*(1+logi.Startbutton)+50*(logi.Ybutton)-50*(logi.Abutton));
gpioServo(ROVM2, 1500+100*(logi.leftjoyX)*(1+logi.Startbutton)-100*(logi.leftjoyY)*(1+logi.Startbutton)+50*(logi.Ybutton)-50*(logi.Abutton));
gpioServo(ROVM3, 1500-100*(logi.leftjoyX)*(1+logi.Startbutton)+100*(logi.leftjoyY)*(1+logi.Startbutton)+50*(logi.Ybutton)-50*(logi.Abutton));
gpioServo(ROVM4, 1500-100*(logi.leftjoyX)*(1+logi.Startbutton)-100*(logi.leftjoyY)*(1+logi.Startbutton)+50*(logi.Ybutton)-50*(logi.Abutton));

gpioServo(HANDCODE, 1500+200*(logi.lefttrigger)*(1+logi.Startbutton)-200*(logi.righttrigger)*(1+logi.Startbutton));
//fidle around till controls are good
gpioServo(ARMPIN1, 1500+logi.rightjoyY*10*(1+logi.Startbutton));
//fidle around till controls are good
gpioServo(ARMPIN2, 1500+logi.rightjoyX*10*(1+logi.Startbutton));
}


int main() {
gpioInitialise();

int fd;
    controller logi_state = {0};
    const char *device_path = "/dev/input/js0"; // MAY NEED TO BE CHANGED

    fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("Error opening joystick");
        exit(EXIT_FAILURE);
    }
controller logi;

gpioSetMode(ROVUP1,PI_OUTPUT);
gpioSetMode(ROVUP2,PI_OUTPUT);

gpioSetMode(ROVM1,PI_OUTPUT);
gpioSetMode(ROVM2,PI_OUTPUT);
gpioSetMode(ROVM3,PI_OUTPUT);
gpioSetMode(ROVM4,PI_OUTPUT);


gpioSetMode(ARMPIN1,PI_OUTPUT);
gpioSetMode(ARMPIN2,PI_OUTPUT);

gpioSetMode(HANDCODE,PI_OUTPUT);


    
/*    int uart_fd;
    struct termios options;
    char buffer[MAX_BUFFER_SIZE];
    int buffer_index = 0;

    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        perror("Error opening UART");
        return -1;
    }

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

    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &options) < 0) {
        perror("Error setting termios settings");
        close(uart_fd);
        return -1;
    }

    printf("Listening for controller data on %s at %d baud...\n", UART_DEVICE, BAUD_RATE);
*/
    while (1) {
        /*
        int bytes_read = read(uart_fd, buffer + buffer_index, MAX_BUFFER_SIZE - 1 - buffer_index);
        if (bytes_read > 0) {
            buffer_index += bytes_read;
            buffer[buffer_index] = '\0';

            char *newline_pos;
            while ((newline_pos = strchr(buffer, '\n')) != NULL) {
                *newline_pos = '\0';

                controller receivedLogi;
                char *token;
                char *rest = buffer;
                int i = 0;
                bool parse_error = false;

                while ((token = strtok_r(rest, ",", &rest)) != NULL) {
                    int value = atoi(token);
                    bool bool_value = (strcmp(token, "true") == 0 || strcmp(token, "1") == 0);

                    switch (i) {
                        case 0: receivedLogi.rightjoyX = value; break;
                        case 1: receivedLogi.rightjoyY = value; break;
                        case 2: receivedLogi.leftjoyX = value; break;
                        case 3: receivedLogi.leftjoyY = value; break;
                        case 4: receivedLogi.lefttrigger = value; break;
                        case 5: receivedLogi.righttrigger = value; break;
                        case 6: receivedLogi.Ybutton = bool_value; break;
                        case 7: receivedLogi.Xbutton = bool_value; break;
                        case 8: receivedLogi.Bbutton = bool_value; break;
                        case 9: receivedLogi.Abutton = bool_value; break;
                        case 10: receivedLogi.Backbutton = bool_value; break;
                        case 11: receivedLogi.Startbutton = bool_value; break;
                        case 12: receivedLogi.Dpadup = bool_value; break;
                        case 13: receivedLogi.Dpadleft = bool_value; break;
                        case 14: receivedLogi.Dpadright = bool_value; break;
                        case 15: receivedLogi.Dpaddown = bool_value; break;
                        default: fprintf(stderr, "Error: Too many elements received: %s\n", buffer); parse_error = true; break;
                    }
                    i++;
                }

                if (!parse_error && i == 16) {
                    logi = receivedLogi; 
                    printf("Received Controller Data:\n");
                    printf("  RX: %d, RY: %d, LX: %d, LY: %d\n", logi.rightjoyX, logi.rightjoyY, logi.leftjoyX, logi.leftjoyY);
                    printf("  LT: %d, RT: %d\n", logi.lefttrigger, logi.righttrigger);
                    printf("  Y: %d, X: %d, B: %d, A: %d\n", logi.Ybutton, logi.Xbutton, logi.Bbutton, logi.Abutton);
                    printf("  Back: %d, Start: %d, Mode: %d\n", logi.Backbutton, logi.Startbutton, logi.Startbutton);
                    printf("  DU: %d, DL: %d, DR: %d, DD: %d\n", logi.Dpadup, logi.Dpadleft, logi.Dpadright, logi.Dpaddown);
                } else if (!parse_error) {
                    fprintf(stderr, "Error: Incorrect number of controller elements received: %s (%d received, expected 17)\n", buffer, i);
                }

                int remaining_len = strlen(newline_pos + 1);
                memmove(buffer, newline_pos + 1, remaining_len);
                buffer_index = remaining_len;
                buffer[buffer_index] = '\0';
            }
        } else if (bytes_read < 0) {
            perror("Error reading from UART");
            break;
        }
        usleep(100);
        */
        if (update_logitech_state(fd, &logi_state) > 0) {
            servomove(logi);
        }
        usleep(50000);
    }

    //close(uart_fd);
    close(fd);
    return 0;
}


