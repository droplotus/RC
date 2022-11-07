// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort, serialPort);
    //ll.serialPort = serialPort;
    if(!strcmp(role, "rx")) { 
        ll.role = LlRx;
    } else if(!strcmp(role, "tx")) {
        ll.role = LlTx;
    } else {
        printf("Error: bad role name\n");
        exit(BAD_ROLE);
    }
    ll.baudRate = baudRate;
    ll.nTries = nTries;
    ll.timeout = timeout;


    if(!llopen(ll)){
        printf("Error: llopen\n");
        exit(1);
    }

    


}
