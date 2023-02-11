#pragma once
#include "MyAvr.h"

/*------------------------------------------------------------------------------
    twim0.h - Twi0, master - avr mega0, tiny0/1, da

    0. uncomment the appropriate set of pins for your mcu
        in twiPins.h
    1. select pins to use- default or alternate
        twim0_stdPins(); //pullups on, portmux set
    2. set baud
        twim0_baud( F_CPU, 100000ul ); //100kHz
    3. turn on, specifying slave address
        twim0_on( 0x44 );
    4. enable interrupts via sei() (avr/interrupts.h)


    optionally set a callback function if not polling via twim0_waitUS()
        twim0_callback( myCallback );

    you can now use one of four functions to move data-

    twim0_writeRead - write wbuffer of wN length, read into rbuffer of rN length
    twim0_writeWrite - write wbuffer of wN length, write wbuffer2 of w2N length
    twim0_write - write wbuffer of wN length (alias to writeRead with no read)
    twim0_read - read into rbuffer of rN length (alias to writeRead with no write)

    if not using a callback function, you can poll for completion-

        twim0_defaultPins();                //init two
        twim0_baud( F_CPU, 100000ul );
        twim0_on( 0x44 );
        sei();

        u8 wbuf[1] = { 0x55 };              //command to read 4 bytes, as an example
        u8 rbuf[4];                         //no need to clear/init read buffer
        twim0_writeRead( wbuf, 1, rbuf, 4 );//write 1 byte (0x55), read 4 bytes

        //blocking until done
        while( twim0_isBusy() ){}           //blocks until done (not a great idea)
        if( twim0_resultOK() ){}            //rbuf has 4 bytes
        else {}                             //was nack'd or bus error/collision

        //blocking until done or a timeout (us)
        if( twim0_waitUS(3000) ){}          //result ok, rbuf has 4 bytes
        else if( twim0_isBusy() ){}         //was timeout, (twim irqs may still be on)
        else {}                             //was nack'd or bus error/collision (twim irqs are off)

        twim0_off();


------------------------------------------------------------------------------*/

typedef void (*twim_callbackT)(void);

void twim0_off          ();
void twim0_on           (u8 address);
bool twim0_isBusy       ();
bool twim0_resultOK     ();
void twim0_callback     (twim_callbackT callbackFunction);
void twim0_writeRead    (const u8* writeBuffer, u16 writeLength, u8* readBuffer, u16 readLength);
void twim0_writeWrite   (const u8* writeBuffer, u16 writeLength, const u8* writeBuffer2, u16 writeLength2);
void twim0_write        (const u8* writeBuffer, u16 writeLength);
void twim0_read         (u8* readBuffer, u16 readLength);
bool twim0_waitUS       (u16 microseconds);
void twim0_stdPins      ();
void twim0_altPins      ();

                        static inline __attribute((always_inline))
void twim0_baud         (uint32_t cpuHz, uint32_t twiHz)
                        {
                        int32_t v = cpuHz/twiHz/2 - 5;
                        TWI0.MBAUD = v >= 0 ? v : 0;
                        }