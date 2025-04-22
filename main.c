#include "gpio.h" // Keep if you're using pigpio for other things
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

#define UART_DEVICE "/dev/ttyS0"
#define BAUD_RATE B115200
#define MAX_BUFFER_SIZE 512 // Increased buffer size to accommodate more data

// ... (GPIO pin definitions) ...

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
    bool Modebutton;
    bool Dpadup;
    bool Dpadleft;
    bool Dpadright;
    bool Dpaddown;
} controller;

controller logi; // Global variable to store controller data

// ... (gpiostart and gpioSetMode calls) ...

// bool servomove(controller logi, bool fastmode); // Function definition

int main() {
    int uart_fd;
    struct termios options;
    char buffer[MAX_BUFFER_SIZE];
    int buffer_index = 0;

    // ... (UART setup code - same as before) ...

    printf("Listening for controller data on %s at %d baud...\n", UART_DEVICE, BAUD_RATE);

    while (1) {
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
                        case 12: receivedLogi.Modebutton = bool_value; break;
                        case 13: receivedLogi.Dpadup = bool_value; break;
                        case 14: receivedLogi.Dpadleft = bool_value; break;
                        case 15: receivedLogi.Dpadright = bool_value; break;
                        case 16: receivedLogi.Dpaddown = bool_value; break;
                        default: fprintf(stderr, "Error: Too many elements received: %s\n", buffer); parse_error = true; break;
                    }
                    i++;
                }

                if (!parse_error && i == 17) {
                    logi = receivedLogi; // Save the received data to the global logi structure
                    printf("Received Controller Data:\n");
                    printf("  RX: %d, RY: %d, LX: %d, LY: %d\n", logi.rightjoyX, logi.rightjoyY, logi.leftjoyX, logi.leftjoyY);
                    printf("  LT: %d, RT: %d\n", logi.lefttrigger, logi.righttrigger);
                    printf("  Y: %d, X: %d, B: %d, A: %d\n", logi.Ybutton, logi.Xbutton, logi.Bbutton, logi.Abutton);
                    printf("  Back: %d, Start: %d, Mode: %d\n", logi.Backbutton, logi.Startbutton, logi.Modebutton);
                    printf("  DU: %d, DL: %d, DR: %d, DD: %d\n", logi.Dpadup, logi.Dpadleft, logi.Dpadright, logi.Dpaddown);
                } else if (!parse_error) {
                    fprintf(stderr, "Error: Incorrect number of controller elements received: %s (%d received, expected 17)\n", buffer, i);
                }

                // Move the remaining part of the buffer
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
    }

    close(uart_fd);
    return 0;
}

// bool servomove(controller logi, bool fastmode){ // Function definition (needs to be called somewhere)
//     // ... (servo control logic using the global 'logi' variable) ...
// }
