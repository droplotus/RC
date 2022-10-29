// Read from serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256

volatile int STOP = FALSE;

void llopen(int fd){
    // Loop for input
    unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    while (STOP == FALSE)
    {
        // Returns after 5 chars have been input
        int bytes = read(fd, buf, BUF_SIZE);
        buf[bytes] = '\0'; // Set end of string to '\0', so we can printf

        //if(buf[0] == 0x7E && buf[1] == 0x03 && buf[2] == 0x03 && buf[3] == 0x03^0x03 && buf[4] == 0x7E)
        if(buf[0] == 0x7E)
            printf("Connection established.\n");
        
        bytes = write(fd, buf, 1);

        buf[2] = 0x07;
        
        sleep(1);
        //bytes = write(fd, buf, BUF_SIZE);
        printf("%d bytes written\n", bytes);
        //printf("BCC1: %d\n", buf[3]);

        // Wait until all bytes have been written to the serial port
        sleep(1);
        break;
    }
    
}

void llread(int fd){
    int aux = 0;
    
    unsigned char buffer[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char
    
    while (STOP == FALSE)
    {
        sleep(3);
        // Returns after 5 chars have been input
        int bytes = read(fd, buffer, BUF_SIZE);
        buffer[bytes] = '\0'; // Set end of string to '\0', so we can printf

        //if(buffer[0] == 0x7E && buffer[1] == 0x03 && buffer[2] == 0x40 && buffer[3] == 0x03^0x40 && buffer[5] == 0x03^0x40 && buffer[6] == 0x7E)
        printf("Coisinho a pegar: %d.\n", buffer[4]);
            

        //buf[2] = 0x07;
        
        //bytes = write(fd, buf, BUF_SIZE);
        //printf("%d bytes written\n", bytes);
        //printf("BCC1: %d\n", buf[3]);

        // Wait until all bytes have been written to the serial port
        // sleep(1);
        break;
    }
}

int main(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];
    
    char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 1;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    llopen(fd);

    llread(fd);

    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
