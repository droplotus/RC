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
static int rest;
static long len;
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
    //unsigned char *buf = (unsigned char *)malloc(5);

    alarmCount = 0;
    int receivedDISC = 0;

    printf("Inside closeTransmitter\n");

    while(alarmCount < ll.nRetransmissions){
        printf("Inside closeTransmitter -> while\n");
        alarm(ll.timeout);
        alarmEnabled = TRUE;

        if (alarmCount > 0)
        {
            alarmCount++;
            printf("Alarm Count = %d\n", alarmCount);
        }

        buffer[0] = FLAG;
        buffer[1] = A;
        buffer[2] = DISC;
        buffer[3] = (A^DISC);
        buffer[4] = FLAG;
        write(fd, buffer, 5);

        while(alarmEnabled && !receivedDISC){
            printf("Inside closeTransmitter -> while -> while\n");
            int bytes = read(fd, buffer, 5);

            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){
                printf("State: %d\n", getState());
                handleMsgByte(buffer[i]);
            }

            if(getC() == DISC) receivedDISC = 1;
            
        }

        if(receivedDISC){
            buffer[0] = FLAG;
            buffer[1] = A;
            buffer[2] = UA;
            buffer[3] = (A^UA);
            buffer[4] = FLAG;
            write(fd, buffer, 5);
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
    //unsigned char *buf = (unsigned char *)malloc(5);
    alarmCount = 0;
    int receivedDISC = 0;
    int receivedUA = 0;

    printf("Inside closeReceiver\n");

    while(alarmCount < ll.nRetransmissions){
        printf("Inside closeReceiver -> while\n");
        alarm(ll.timeout);
        alarmEnabled = TRUE;

        if (alarmCount > 0)
        {
            alarmCount++;
            printf("Alarm Count = %d\n", alarmCount);
        }

        while(alarmEnabled && !receivedDISC){
            printf("Inside closeReceiver -> while -> while\n");
            int bytes = read(fd, buffer, 5);
            printf("Inside closeReceiver -> while -> while -> bytes\n");
            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){
                handleMsgByte(buffer[i]);
            }

            if(getC() == DISC) receivedDISC = 1;
            
        }

        buffer[0] = FLAG;
        buffer[1] = A;
        buffer[2] = DISC;
        buffer[3] = (A^DISC);
        buffer[4] = FLAG;
        write(fd, buffer, 5);
        
        while(!receivedUA){
            setStatee(START);
            int bytes = read(fd, buffer, 5);

            if(bytes < 0) return -1;

            for (int i = 0; i<bytes; i++){
                handleMsgByte(buffer[i]);
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

unsigned char *getData(unsigned char *buf, int size){
    unsigned char *buff = (unsigned char *)malloc(size * sizeof(char));

    for(int i=4; i<size+5; i++){
        buff[i-4] = buf[i];
    }

    printf("%li\n", sizeof(buff));
    return buff;
}

unsigned char *createInformationFrame(unsigned char *buf, int filelen, int size){
    unsigned char *buff = (unsigned char *)malloc((size+6) * sizeof(char));

    buff[0] = FLAG;
    buff[1] = A;
    buff[2] = C;
    buff[3] = (A^C);
    buff[size+5] = (A^C);
    buff[size+6] = FLAG;

    for(int i=4; i<size+5; i++){
        buff[i] = buf[i-4];
    }

    

    //printf("size: %i, Inicio: Compare buff: %d, buf: %d\n", size, buff[size+4], buf[size]);
    //printf("size: %i, Fim: Compare buff: %d, buf: %d\n", size, buff[size+3], buf[size-1]);

    return buff;
}

void sendPacket(unsigned char *buf, int size) {
    //buf = createInformationFrame(buf, filelen, size);
    write(fd, buf, size);

}

void sendFile(const char *filename) {
    FILE *fileptr;
    long filelen;
    unsigned char *buf;
    unsigned char *send;

    unsigned int index = 0;

    fileptr = fopen("../penguin.gif", "rb");

    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);

    printf("%li\n", filelen);

    len = filelen;

    buf = (unsigned char *)malloc(filelen * sizeof(char));
    fread(buf, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    rest = filelen%1000;

    while(filelen > 0) {
        printf("file size: %ld\n", filelen);
        if(filelen >= 1000){
            send = (unsigned char *)malloc(1000 * sizeof(char));
            for(int i=0; i<1000; i++){
                send[i] = buf[i+index*1000];
            }

            sendPacket(send, 1000);
            index++;
            filelen -= 1000;
        }
        else{
            send = (unsigned char *)malloc(filelen * sizeof(char));
            for(int i=0; i<filelen; i++){
                send[i] = buf[i+index*1000];
            }
            sendPacket(send, filelen);
            filelen = 0;
            printf("finished sending file\n");
        }
    }
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const char *filename)
{
    /*unsigned char buf[1007] = {0};

    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = C;
    buf[3] = 1;
    buf[1005] = 1;
    buf[1006] = FLAG;

    for(int i=4; i<1005; i++)
        buf[i] = i;*/

    printf("Entering llwrite\n");
    

    sendFile(filename);

    printf("Returning llwrite\n");

    return 1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread()
{
    // int aux = 0;
    
    //unsigned char *buf;
    //unsigned char *buff;

    unsigned char bufferr[1000] = {0};
    //unsigned char bufferrest[rest] = {0};

    FILE *fileptr;
    // int mode = O_WRONLY | O_APPEND | O_CREAT;
    fileptr = fopen("newpinguim.gif", "a+");
    

    while (TRUE)
    {
        //buf = (unsigned char *)malloc((b+7) * sizeof(char)); // +1: Save space for the final '\0' char

        // Returns after 5 chars have been input
        int bytes = read(fd, bufferr, 1000);
        printf("bytes: %d\n", bytes);

        //buff = getData(buf, b);
        //printf("DEBUG: %d : %d\n", buff[b-1], buf[bytes-4]);

        //if(buffer[0] == 0x7E && buffer[1] == 0x03 && buffer[2] == 0x40 && buffer[3] == 0x03^0x40 && buffer[5] == 0x03^0x40 && buffer[6] == 0x7E)
        
        for(int i=0; i<bytes; i++)
            fprintf(fileptr, "%c", bufferr[i]);
            
        
        memset(bufferr,0,sizeof(bufferr));
        if(bytes < 1000) break;
        //buf[2] = 0x07;
        
        //bytes = write(fd, buf, BUF_SIZE);
        //printf("%d bytes written\n", bytes);
        //printf("BCC1: %d\n", buf[3]);

        // Wait until all bytes have been written to the serial port
        // sleep(1);
    }

    fclose(fileptr);

    printf("Returning llread\n");

    return 1;
}

void restore(){
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
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
