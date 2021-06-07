#pragma once
/*------------------------------------------------------------------------------
    twis.h - Twi slave

    1. select pins to use- default or alternate
    twis_defaultPins();
    2. init with address and callback function
    twis_init( 0x40, myCallback);
    3. enable interrupts via sei() (avr/interrupts.h)

    optional - set a 2nd address, or an address mask, can be set at anytime


    callback function, isr provides state and status register
    (status register not really necessary, but can be used get more info on
     when in TWIS_ERROR state)

    bool myCalback(twis_irqstate_t state, u8 statusReg){
        you have the enum states to deal with as needed
        return true if everything ok, false if you want to stop the transaction
        you have 3 functions to use with twis-
            twis_lastAddress - when in TWIS_ADDRESSED, this will give address
                               seen to get here (could be 0, address, address2,
                               or a mask match)
            twis_write - when in TWIS_MREAD, you can reply with a write
            twis_read - when in TWIS_MWRITE, you can read what was sent
    }

    NOTE- gencall is enabled by default, so check the address in the callback
          when in TWIS_ADDRESSED state (simply enabled to eliminate one more
          option to set- most likely never seen but if so you can ignore in the
          callback by only returning true when dealing with an address you
          want to respond to)
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
void twis_addressMask   (u8 SlaveAddressMask); //no 2nd address
void twis_init          (u8 SlaveAddress, twis_callback_t callbackFunction);
void twis_defaultPins   ();
void twis_altPins       ();


