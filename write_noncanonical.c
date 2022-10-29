// Write to serial port in non-canonical mode
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


// Defining flags and stuff
#define FLAG 0x7E
#define A 0x03
#define C 0x03
#define DISC 0x0B
#define UA 0x07

typedef enum {TRANSMITTER, RECEIVER}Job;

typedef enum {

	START,
	FLAG_RCV,
	A_RCV,
	C_RCV,
	BCC_OK,
	STOP

}State;

//volatile int STOP = FALSE;

int llopen(int porta, Job j){

	//fsync(porta);

	// Create string to send
  unsigned char writing[1] = {0};
  unsigned char reading[1] = {0};
  
  //buf[0] = FLAG;
  //buf[1] = A;
  //buf[2] = C;
  //buf[3] = A^C;
  //buf[4] = FLAG;
  
  int bytes;
  
	if(j == TRANSMITTER){
		printf("Entering transmitter work.\n");
		
		State s = START;
		printf("Starting...\n");
		while(1){
		
			switch (s) {
				case START:
					writing[0] = FLAG;
					bytes = write(porta, writing, 1);
					sleep(1);
					bytes = read(porta, reading, 1);
					//printf("%d", bytes);
					if(reading[0] == FLAG){
						s = FLAG_RCV;
						printf("FLAG_RCV\n");
					}
					break;
				case FLAG_RCV:
					writing[0] = A;
					bytes = write(porta, writing, 1);
					sleep(1);
					bytes = read(porta, reading, 1);
					if(reading[0] == A){
						s = A_RCV;
						printf("A_RCV\n");
					}
					break;
				case A_RCV:
					writing[0] = C;
					bytes = write(porta, writing, 1);
					sleep(1);
					bytes = read(porta, reading, 1);
					if(reading[0] == UA){
						s = C_RCV;
						printf("C_RCV\n");
					}
					break;
				case C_RCV:
					writing[0] = A^C;
					bytes = write(porta, writing, 1);
					sleep(1);
					bytes = read(porta, reading, 1);
					if(reading[0] == A^C) {
						s = BCC_OK;
						printf("BCC_OK\n");
					}
					break;
				case BCC_OK:
					writing[0] = FLAG;
					bytes = write(porta, writing, 1);
					sleep(1);
					bytes = read(porta, reading, 1);
					if(reading[0] == FLAG) {
						s = STOP;
						printf("STOP\n");
					}
					break;
				case STOP:
					break;
				default:
					break;
			}
			if (s==STOP) break;
		}
		printf("Stopping...\n");
		
  }else {
    printf("Entering receiver work.\n");
    while(1){
			
		}
  }
  return porta;
}

int lllopen(int fd){
    // Create string to send
    unsigned char buf[255] = {0};

    printf("Starding llopen\n");
    
    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = C;
    buf[3] = A^C;
    buf[4] = FLAG;

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[5] = '\n';

    int bytes = write(fd, buf, 5);
    printf("%d bytes written\n", bytes);
    //printf("BCC1: %d\n", buf[3]);

    // Wait until all bytes have been written to the serial port
    sleep(1);


    // Start receiving confirmation 

   
		//fflush(fd);
    
    // Loop for input
    //unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    while (STOP == FALSE)
    {
        // Returns after 5 chars have been input
        bytes = read(fd, buf, BUF_SIZE);
        buf[bytes] = '\0'; // Set end of string to '\0', so we can printf

        if(buf[0] == 0x7E)
            //printf("Flag received.\n");
        if(buf[1] == 0x03)
            //printf("A received.\n");
        if(buf[2] == 0x07)
            //printf("C received.\n");
        if(buf[3] == 0x03^0x07)
            //printf("BCC received.\n");
        if(buf[4] == 0x7E)
            //printf("Flag received.\n");
        break;

    }

    printf("Ending llopen\n");

    // Falta alarmes
}


int llwrite(int fd) {
    unsigned char buf[255] = {0};

    printf("Entering llwrite\n");
    
    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = 0x40;
    buf[3] = A^C;
    //printf("BCC1: %d", BCC1);
    buf[4] = 24;
    buf[5] = A^C;
    buf[6] = FLAG;

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[7] = '\0';

    int bytes = write(fd, buf, BUF_SIZE);
    printf("%d bytes written\n", bytes);
    
    printf("hello: %d\n", fd);
    printf("hello\n");
    //printf("BCC1: %d\n", buf[3]);

    // Wait until all bytes have been written to the serial port
    sleep(1);


    // Start receiving confirmation 

    
    // Loop for input
    //unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    // while (STOP == FALSE)
    // {
    //     // Returns after 5 chars have been input
    //     bytes = read(fd, buf, BUF_SIZE);
    //     buf[bytes] = '\0'; // Set end of string to '\0', so we can printf

    //     if(buf[0] == 0x7E)
    //         printf("Flag received.\n");
    //     if(buf[1] == 0x03)
    //         printf("A received.\n");
    //     if(buf[2] == 0x07)
    //         printf("C received.\n");
    //     if(buf[3] == 0x03^0x07)
    //         printf("BCC received.\n");
    //     if(buf[4] == 0x7E)
    //         printf("Flag received.\n");
    //     break;

    // }
}


int llclose(int fd){
    // Create string to send
    unsigned char buf[255] = {0};

    printf("Starding llclose\n");

    printf("Sending disc\n");
    
    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = DISC;
    buf[3] = A^DISC;
    buf[4] = FLAG;

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[5] = '\n';

    int bytes = write(fd, buf, BUF_SIZE);
    printf("%d bytes written\n", 6);
    //printf("BCC1: %d\n", buf[3]);

    // Wait until all bytes have been written to the serial port
    sleep(1);


    // Start receiving confirmation 

    //fflush(fd);

    printf("Waiting for disc\n");

    
    // Loop for input
    //unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    while (STOP == FALSE)
    {
        // Returns after 5 chars have been input
        bytes = read(fd, buf, BUF_SIZE);
        buf[bytes] = '\0'; // Set end of string to '\0', so we can printf

        if(buf[0] == 0x7E)
            //printf("Flag received.\n");
        if(buf[1] == 0x03)
            //printf("A received.\n");
        if(buf[2] == 0x0B)
            //printf("C received.\n");
        if(buf[3] == 0x03^0x0B)
            //printf("BCC received.\n");
        if(buf[4] == 0x7E)
            //printf("Flag received.\n");
        break;

    }

    printf("Sending UA\n");


    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = UA;
    buf[3] = A^UA;
    buf[4] = FLAG;

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[5] = '\n';

    bytes = write(fd, buf, BUF_SIZE);
    printf("%d bytes written\n", 6);
    //printf("BCC1: %d\n", buf[3]);

    // Wait until all bytes have been written to the serial port
    sleep(1);

    printf("Ending llopen\n");

    return 0;

    // Falta alarmes
}


int main(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing, and not as controlling tty
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

    llopen(fd, TRANSMITTER);

	return 0;

    llwrite(fd);

    //llclose(fd);


    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}



