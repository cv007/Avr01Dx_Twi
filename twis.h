#pragma once
/*------------------------------------------------------------------------------
    twis.h - Twi slave

    1. select pins to use- default or alternate
    twis_defaultPins();
    2. init with address and callback function
    twis_init( 0x40, myCallback);

    callback function, isr provides state and status register
    (status register not really necessary, but can use for trouvleshooting)

    bool myCalback(twis_irqstate_t state, u8 statusReg){
        you have the enum states to deal with as needed
        return true if everything ok, false if you want to stop the transaction
        you have 3 functions to deal with twis- _read, _write, _lastAddress
            twis_lastAddress - when in TWIS_ADDRESSED, this will give address
                               (in case 2 addresses are used)
            twis_write - when in TWIS_MREAD, you can reply with a write
            twis_read - when in TWIS_MWRITE, you can read what was sent
    }
------------------------------------------------------------------------------*/
#include "MyAvr.h"

typedef enum { TWIS_ADDRESSED, TWIS_MREAD, TWIS_MWRITE, TWIS_STOPPED,
               TWIS_ERROR } twis_irqstate_t;
typedef bool(*twis_callback_t)(twis_irqstate_t state, u8 statusReg);

void twis_off           ();
void twis_write         (u8 value);
u8   twis_read          ();
u8   twis_lastAddress   ();
void twis_address2      (u8 SlaveAddress2);
void twis_init          (u8 SlaveAddress, twis_callback_t callbackFunction);
void twis_defaultPins   ();
void twis_altPins       ();


