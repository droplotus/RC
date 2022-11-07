// Link layer protocol implementation

#include "link_layer.h"
#include "state_machine.h"

#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>


// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source


static struct termios oldtio;
struct termios newtio;

static int fd;
unsigned char buffer[5];
clock_t start, end;

// Alarm Setup
int alarmEnabled = FALSE;
int alarmCount = 0;

LinkLayer ll;


// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
}



////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{

    ll = connectionParameters;

    fd = open(ll.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(ll.serialPort);
        printf("Error: fd<0\n");
        exit(-1);
    }

     // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = (ll.baudRate) | CS8 | CLOCAL | CREAD; // tentar usar o baudRate do parametro
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;


    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 10*ll.timeout; // Inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // Blocking read until 5 chars received

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");

        exit(-1);
    }

    (void)signal(SIGALRM, alarmHandler);

    switch(ll.role){
        case LlTx:
            {
                if(!openTransmitter()){
                    printf("Error: openTransmitter\n");
                    exit(1);
                }

                return 1;
            }
        case LlRx:
            {
                if(!openReceiver()){
                    printf("Error: openReceiver\n");
                    exit(1);
                }

                return 1;
            }
        default:
            return 0;

    }

    return 1;
}

int openTransmitter() {
    alarmCount = 0;

    while(alarmCount < ll.nRetransmissions){
        alarm(ll.timeout);
        alarmEnabled = TRUE;

        if (alarmCount > 0)
        {
            alarmCount++;
            printf("Alarm Count = %d\n", alarmCount);
        }

        buffer[0] = FLAG;
        buffer[1] = A;
        buffer[2] = C;
        buffer[3] = (A^C);
        buffer[4] = FLAG;
        write(fd, buffer, 5);
        
        while(alarmEnabled && !(getState() == STOP)){
            int bytes = read(fd, buffer, 5);

            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){
                handleMsgByte(buffer[i]);
            }
            
        }
        if(getState() == STOP){
                printf("(OPEN) Tudo perfeito do transmissor.\n");
                break;
        }
    }


    return 1;
}

int closeTransmitter() {
    setStatee(START);
    unsigned char buf[5];
    alarmCount = 0;
    int receivedDISC = 0;

    while(alarmCount < ll.nRetransmissions){
        alarm(ll.timeout);
        alarmEnabled = TRUE;

        if (alarmCount > 0)
        {
            alarmCount++;
            printf("Alarm Count = %d\n", alarmCount);
        }

        buf[0] = FLAG;
        buf[1] = A;
        buf[2] = DISC;
        buf[3] = (A^DISC);
        buf[4] = FLAG;
        write(fd, buf, 5);
        
        while(alarmEnabled && !receivedDISC){
            int bytes = read(fd, buf, 5);
            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){
                handleMsgByte(buf[i]);
            }

            if(getC() == DISC) receivedDISC = 1;
            
        }

        if(receivedDISC){
            buf[0] = FLAG;
            buf[1] = A;
            buf[2] = UA;
            buf[3] = (A^UA);
            buf[4] = FLAG;
            write(fd, buf, 5);
            printf("(CLOSE) Tudo perfeito do transmissor.\n");
            break;
        }
    }


    return 1;
}

int openReceiver() {
    while(TRUE){
        int bytes = read(fd, buffer, 5);

        if(bytes < 0) return -1;

        for (int i = 0; i<bytes; i++){
            handleMsgByte(buffer[i]);
        }

        if(getState() == STOP){
            printf("(OPEN) Tudo perfeito do recetor.\n");
            break;
        }
    }
    
    buffer[0] = FLAG;
    buffer[1] = A;
    buffer[2] = UA;
    buffer[3] = (A^UA);
    buffer[4] = FLAG;
    write(fd, buffer, 5);

    return 1;
}

int closeReceiver() {
    setStatee(START);
    unsigned char buf[5];
    alarmCount = 0;
    int receivedDISC = 0;
    int receivedUA = 0;

    while(alarmCount < ll.nRetransmissions){
        alarm(ll.timeout);
        alarmEnabled = TRUE;

        if (alarmCount > 0)
        {
            alarmCount++;
            printf("Alarm Count = %d\n", alarmCount);
        }

        while(alarmEnabled && !receivedDISC){
            int bytes = read(fd, buf, 5);

            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){

                handleMsgByte(buf[i]);
            }

            if(getC() == DISC) receivedDISC = 1;
            
        }

        buf[0] = FLAG;
        buf[1] = A;
        buf[2] = DISC;
        buf[3] = (A^DISC);
        buf[4] = FLAG;
        write(fd, buf, 5);
        
        while(!receivedUA){
            setStatee(START);
            int bytes = read(fd, buf, 5);

            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){
                handleMsgByte(buf[i]);
            }
            
            if(getC() == UA) receivedUA = 1;
        }

        if(receivedUA){
            printf("(CLOSE) Tudo perfeito do recetor.\n");
            break;
        }
    }


    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

void restore(){
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    switch(ll.role){
        case LlTx:
            {
                if(!closeTransmitter()){
                    printf("Error: openTransmitter\n");
                    exit(1);
                }
                restore();
                return 1;
            }
        case LlRx:
            {
                if(!closeReceiver()){
                    printf("Error: openReceiver\n");
                    exit(1);
                }
                restore();
                return 1;
            }
        default:
            return 0;

    }

    return 1;
}
