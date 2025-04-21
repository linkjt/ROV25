#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSIZE 1024
#define SERVER_PORT 5005
// defined pins, change this later


//MAKE IT FOR ROV X and Z 1 and two to be able to move on both axis, thus finishing all movement
// look at transfer ethernet code

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
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFSIZE];
    ssize_t recv_len;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("YOUR_RASPBERRY_PI_IP"); // Use your Pi's static IP
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for UDP packets on %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    while (1) {
        recv_len = recvfrom(sockfd, buffer, BUFSIZE - 1, 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[recv_len] = '\0'; // Null-terminate the received data
        printf("Received message from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
        fastmode = servomove(logi,fastmode);
    }

    close(sockfd);
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


