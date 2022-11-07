#include <unistd.h>
#include <stdio.h>
#include "state_machine.h"

static void setState(state_t new_state);

static void handleStart(byte msg_byte);
static void handleFlagReceived(byte msg_byte);
static void handleAReceived(byte msg_byte);
static void handleCReceived(byte msg_byte);
static void handleBccOk(byte msg_byte);

static state_machine_ctrl_st state_machine = (state_machine_ctrl_st){
    .current_state = START
};

state_t getState() {
    return state_machine.current_state;
}

byte getC() {
    return state_machine.ctrl;
}

void handleMsgByte(byte msg_byte) {
    switch (state_machine.current_state) {
        case START:
            handleStart(msg_byte);
            break;
        case FLAG_RCV:
            handleFlagReceived(msg_byte);
            break;
        case A_RCV:
            handleAReceived(msg_byte);
            break;
        case C_RCV:
            handleCReceived(msg_byte);
            break;
        case BCC_OK:
            handleBccOk(msg_byte);
            break;
        case STOP:
            return;
    }
}

static void handleStart(byte msg_byte) {
    switch(msg_byte) {
        case FLAG:
            setState(FLAG_RCV);
            break;
        default:
            break;
    }
}

static void handleFlagReceived(byte msg_byte) {
    switch (msg_byte) {
        case FLAG:
            break;
        case A:
            setState(A_RCV);
            state_machine.addr = msg_byte;
            break;
        default:
            setState(START);
            break;
    }
}

static void handleAReceived(byte msg_byte) {

    switch (msg_byte) {
        case FLAG:
            setState(FLAG_RCV);
            break;
        case DISC:
            setState(C_RCV);
            state_machine.ctrl = msg_byte;
            break;
        case UA:
            setState(C_RCV);
            state_machine.ctrl = msg_byte;
            break;
        case C:
            setState(C_RCV);
            state_machine.ctrl = msg_byte;
            break;
        default:
            setState(START);
            break;    
    }
}

static void handleCReceived(byte msg_byte) {
    switch (msg_byte) {
        case FLAG:
            setState(FLAG_RCV);
            break;
        default:
            if(msg_byte == (A^C) || msg_byte == (A^UA) || msg_byte == (A^DISC)) {
                setState(BCC_OK);
            } else {
                setState(START);
            }
            break;    
    }
}

static void handleBccOk(byte msg_byte) {
    switch (msg_byte) {
        case FLAG:
            setState(STOP);
            break;
        default:
            setState(START);
            break;
    }
}

static void setState(state_t new_state) {
    state_machine.current_state = new_state;
}

void setStatee(state_t new_state) {
    setState(new_state);
}