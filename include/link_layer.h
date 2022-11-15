// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

typedef unsigned char byte;

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
    int nTries;
} LinkLayer;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer connectionParameters);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
//int llwrite(const unsigned char *buf, int bufSize);
int llwrite(const char *filename);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
//int llread(unsigned char *packet);
int llread();
// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int showStatistics);

int openTransmitter();
int openReceiver();
int closeTransmitter();
int closeReceiver();

int stuffing(unsigned char * buffer, int startingByte, int lenght, unsigned char * stuffedMessage);
int destuffing(unsigned char * buffer, int startingByte, int lenght, unsigned char * destuffedMessage);

unsigned char *createInformationFrame(unsigned char *buf, int filelen, int size);
void sendPacket(unsigned char *buf, int size);
void sendFile(const char *filename);

void restore();

#endif // _LINK_LAYER_H_
