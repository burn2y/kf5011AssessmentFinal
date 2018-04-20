/* Game Controller */
#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>
#include <stdio.h>
#include <FXOS8700Q.h>
#include <C12832.h>
#include <string>

C12832 lcd(D11, D13, D12, D7, D10);

/* event queue and thread support */
Thread dispatch;
EventQueue periodic;

/* Input from Potentiometers */
AnalogIn  left(A0);
AnalogIn right(A1);

/* User input states */
int throttle;
double orientation;

//Socket addresses
SocketAddress lander("192.168.1.161",65200);
SocketAddress dash("192.168.1.161",20770);

EthernetInterface eth;
UDPSocket udp;

/* States from Lander */
char altitude [50];
char feul [50];
char flying [50];
char crashed [50];
char orientation1 [50];
char velocityX [50];
char velocityY [50];


int send(char *message, size_t s, SocketAddress dest)
{
  nsapi_size_or_error_t r = udp.sendto(dest,message, s);

  return r;
}

/* Task for asynchronous UDP communications with dashboard */
void dashboard(void)
{
  char dashBuffer[1024];

  printf("Orientation%s\n", orientation1); // ... but this line fails to print

  send(dashBuffer, strlen(dashBuffer), dash);
}

/* Task for synchronous UDP communications wi lander */
void communications(void)
{
    SocketAddress source;

    char throttleString [13];
    itoa (throttle, throttleString, 10);

    char orientationString[10];
    if(orientation > 0)
    {
      sprintf(orientationString, "+%.1lf" , orientation);
    }
    else
    {
      sprintf(orientationString, "%.1lf" , orientation);
    }

    char landerMessage [1024], line[80];
    landerMessage[0] = '\0';

    strcat(landerMessage, "command:!\n");
    sprintf(line , "throttle:%s\nroll:%s\n", throttleString, orientationString);
    strcat(landerMessage, line);

    printf("%s", landerMessage);
    send(landerMessage, strlen(landerMessage), lander);
    printf("Message sent...\n");

    char buffer[1024];
    int len = NULL;

    while(len == NULL)
    {
      printf("Listening for response...\n");
      len = udp.recvfrom(&source,buffer,
                                     sizeof(buffer));
    }

    printf("Lander has responded!\n");
    buffer[len] = '\0';
    printf("%s\n", buffer);

    char line1 [50];
    char line2 [50];
    char line3 [50];
    char line4 [50];
    char line5 [50];
    char line6 [50];
    char line7 [50];
    char line8 [50];

    char * tok;

    tok = strtok(buffer, "\n");
    strcpy(line1, tok);
    printf("%s\n", line1);

    tok = strtok(NULL, "\n");
    strcpy(line2, tok);

    printf("%s\n", line2);

    tok = strtok(NULL, "\n");
    strcpy(line3, tok);
    printf("%s\n", line3);

    tok = strtok(NULL, "\n");
    strcpy(line4, tok);
    printf("%s\n", line4);

    tok = strtok(NULL, "\n");
    strcpy(line5, tok);
    printf("%s\n", line5);

    tok = strtok(NULL, "\n");
    strcpy(line6, tok);
    printf("%s\n", line6);

    tok = strtok(NULL, "\n");
    strcpy(line7, tok);
    printf("%s\n", line7);

    tok = strtok(NULL, "\n");
    strcpy(line8, tok);
    printf("%s\n", line8);

    buffer[0] = '\0';

    tok = strtok(line2, ":");
    tok = strtok(NULL, "\n");
    strcpy(altitude, tok);

    tok = strtok(line3, ":");
    tok = strtok(NULL, "\n");
    strcpy(feul, tok);

    tok = strtok(line4, ":");
    tok = strtok(NULL, "\n");
    strcpy(flying, tok);

    tok = strtok(line5, ":");
    tok = strtok(NULL, "\n");
    strcpy(crashed, tok);

    tok = strtok(line6, ":");
    tok = strtok(NULL, "\n");
    strcpy(orientation1, tok);

    tok = strtok(line7, ":");
    tok = strtok(NULL, "\n");
    strcpy(velocityX, tok);

    tok = strtok(line8, ":");
    tok = strtok(NULL, "\n");
    strcpy(velocityY, tok);

    printf("Orientation%s\n", orientation1); //This line is printed...

    dashboard(); // dashboard function called. dashboard function on line 50
}

/* Task for polling sensors */
void user_input(void){

    throttle = (int)round(right.read() * 100);
    orientation = (left.read() * 2) - 1;
    communications();
}

int main() {
    eth.connect();
    const char *ip = eth.get_ip_address();
    printf("IP Address is: %s\n", ip ? ip : "No IP" );
    udp.open(&eth);


    printf("lander is on %s/%d\n",lander.get_ip_address(),lander.get_port() );
    printf("dash   is on %s/%d\n",dash.get_ip_address(),dash.get_port() );

    user_input();

    /* periodic tasks */
    /*TODO call periodic tasks;
            communications, user_input, dashboard
            at desired rates.

      periodic.call_every(<time in ms>, <function to call>);
    */

    /* start event dispatching thread */
    //dispatch.start( callback(&periodic, &EventQueue::dispatch_forever) );

    while(1) {
        /* update display at whatever rate is possible */
        /*TODO show user information on the LCD */
        /*TODO set LEDs as appropriate to show boolean states */

        //wait(1);
        /*TODO you may want to change this time
                    to get a responsive display

Example Output:

IP Address is: 192.168.1.158
lander is on 192.168.1.161/65200
dash   is on 192.168.1.161/20770
command:!
throttle:100
roll:+1.0
Message sent...
Listening for response...
Lander has responded!
command:=
altitude:464.5519000000001
fuel:100.0
flying:1
crashed:0
orientation:294.0
Vx:10.0
Vy:17.937999999999953

command:=
altitude:464.5519000000001
fuel:100.0
flying:1
crashed:0
orientation:294.0
Vx:10.0
Vy:17.937999999999953
Orientation294.0

*/
    }

}
