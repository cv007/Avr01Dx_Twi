#pragma once
/*------------------------------------------------------------------------------
    twim.h - Twi0, master - avr mega0, tiny0/1, da

    1. select pins to use- default or alternate
        twim_defaultPins(); //pullups on, portmux set
    2. set baud
        twim_baud( F_CPU, 100000ul ); //100kHz
    3. turn on, specifying slave address
        twim_on(0x44);
    4. enable interrupts via sei() (avr/interrupts.h)


    optionally set a callback function if not polling via twim_isBusy() or
    twim_waitUS()
        twim_callback(myCallback);

    you can now use one of four functions to move data-

    twim_writeRead - write wbuffer of wN length, read into rbuffer of rN length
    twim_writeWrite - write wbuffer of wN length, write wbuffer2 of w2N length
    twim_write - write wbuffer of wN length (alias to writeRead with no read)
    twim_read - read into rbuffer of rN length (alias to writeRead with no write)

    if not using a callback function, you can poll for completion-

        u8 wbuf[1] = { 0x55 }; //command to read 4 bytes, as an example
        u8 rbuf[4]; //no need to clear/init
        twim_writeRead( wbuf, 1, rbuf, 4 );

        //blocking until done
        while( twim_isBusy() ){} //blocks until done
        if( twim_lastResultOK() ) //rbuf has 4 bytes
        else //was nack'd or bus error/collision

        //or use a timeout in us
        if( twim_waitUS(3000) ) //rbuf has 4 bytes
        else if( twim_isBusy() ) //was timeout, (twim irqs may still be on)
        else //was nack'd or bus error/collision (twim irqs are off)

        twim_off();


------------------------------------------------------------------------------*/
#include "MyAvr.h"

typedef void (*twim_callbackT)(void);

void twim_off           ();
void twim_on            (u8 address);
bool twim_isBusy        ();
bool twim_lastResultOK  ();
void twim_callback      (twim_callbackT callbackFunction);
void twim_writeRead     (const u8* writeBuffer, u16 writeLength, u8* readBuffer, u16 readLength);
void twim_writeWrite    (const u8* writeBuffer, u16 writeLength, const u8* writeBuffer2, u16 writeLength2);
void twim_write         (const u8* writeBuffer, u16 writeLength);
void twim_read          (u8* readBuffer, u16 readLength);
bool twim_waitUS        (u16 microseconds);
void twim_defaultPins   ();
void twim_altPins       ();

                        __attribute((always_inline)) inline static
void twim_baud          (uint32_t cpuHz, uint32_t twiHz)
                        {
                        int32_t v = cpuHz/twiHz/2 - 5;
                        TWI0.MBAUD = v >= 0 ? v : 0;
                        }
