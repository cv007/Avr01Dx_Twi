#pragma once
#include "MyAvr.h"

/*------------------------------------------------------------------------------
    twis0.h - Twi0 slave

    1. uncomment the appropriate set of pins for your mcu
        in twiPins.h to select the twi pins to use
    2. turn on with address and callback function
        twis0_on( 0x40, myCallback);
    3. enable interrupts via sei() (avr/interrupts.h)

    optional - set a 2nd address, or an address mask, can be set at anytime


    callback function, isr provides state and status register
    (status register not really necessary, but can be used get more info on
     when in TWIS_ERROR state)

    bool myCalback(twis_irqstate_t state, u8 statusReg){
        you have the enum states to deal with as needed
        return true if everything ok, false if you want to stop the transaction
        you have 3 functions to use with twis0-
            twis0_lastAddress() - when in TWIS_ADDRESSED, this will give address
                               seen to get here (could be 0, address, address2,
                               or a mask match)
            twis0_write() - when in TWIS_MREAD, you can reply with a write
            twis0_read() - when in TWIS_MWRITE, you can read what was sent
    }

    NOTE- gencall is enabled by default, so check the address in the callback
          when in TWIS_ADDRESSED state (simply enabled to eliminate one more
          option to set- most likely never seen but if so you can ignore in the
          callback by only returning true when dealing with an address you
          want to respond to)
------------------------------------------------------------------------------*/


typedef enum { TWIS_ADDRESSED, TWIS_MREAD, TWIS_MWRITE, TWIS_STOPPED, TWIS_ERROR } twis_irqstate_t;
typedef bool(*twis_callback_t)(twis_irqstate_t state, u8 statusReg);

void twis0_on           (u8 SlaveAddress, twis_callback_t callbackFunction);
void twis0_off          ();
void twis0_write        (u8 value);
u8   twis0_read         ();
u8   twis0_lastAddress  ();
void twis0_address2     (u8 SlaveAddress2);
void twis0_addressMask  (u8 SlaveAddressMask); //no 2nd address when using this option

