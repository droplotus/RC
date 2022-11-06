// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    ll.serialPort = serialPort;
    if(role == "rx") { 
        ll.role = LlRx;
    } else if(role == "tx") {
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
