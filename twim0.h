                #pragma once

                //======================================================================
                //  twim0.h - Twi0, master - avr mega0, tiny0/1
                //======================================================================

                #include "MyAvr.h"


                /*------------------------------------------------------------------------------
                    1. in twiPins.h, create a define at end of file to 
                        init the pins (see end of twiPins.h for examples)
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

                        twim0_baud( F_CPU, 100000ul );
                        twim0_on( 0x44 );
                        sei();

                        u8 wbuf[1] = { 0x55 };              //command to read 4 bytes, as an example
                        u8 rbuf[4];                         //no need to clear/init read buffer
                        twim0_writeRead( wbuf, 1, rbuf, 4 );//write 1 byte (0x55), read 4 bytes

                        //blocking until done or a timeout (us)
                        twim_state_t res = twim0_waitUS(3000);
                        if( res == TWIM_OK ){}              //result ok, rbuf has 4 bytes
                        else if( res == TWIM_TIMEOUT ){     //was timeout, (twim irqs may still be on)
                            twim0_bus_recovery();
                            }
                        else {                              //was nack'd or bus error/collision (twim irqs are off)

                            }
                        twim0_off();                        //turn off (back to TWIM_OFF state)

                        NOTE: FM+ mode is always used but if do not want it you can modify
                            the twim0_on() function
                ------------------------------------------------------------------------------*/

                // off (twi is off) -> ready (twi is on) -> busy (transaction started, twi irq's on)
                // -> ok (transaction completed without error, twi irq's are off)
                // -> timeout (blocking/polling timed out, do twim0_bus_recovery)
                // -> error (transaction had an error- nack, bus collision, etc., do twim0_bus_recovery)
                typedef enum { TWIM_OFF, TWIM_READY, TWIM_BUSY, TWIM_OK, TWIM_TIMEOUT, TWIM_ERROR } 
twim_state_t;

                typedef void (*twim_callbackT)(void);

                void 
twim0_on        (u8 address);
                void 
twim0_off       ();
                bool 
twim0_isBusy    ();
                twim_state_t 
twim0_state     ();
                void 
twim0_callback  (twim_callbackT callbackFunction);
                void 
twim0_writeRead (const u8* writeBuffer, u16 writeLength, u8* readBuffer, u16 readLength);
                void 
twim0_writeWrite(const u8* writeBuffer, u16 writeLength, const u8* writeBuffer2, u16 writeLength2);
                void 
twim0_write     (const u8* writeBuffer, u16 writeLength);
                void 
twim0_read      (u8* readBuffer, u16 readLength);
                twim_state_t 
twim0_waitUS    (u16 microseconds);
                void 
twim0_bus_recovery();

                //inline code to get compile time computation
                __attribute((always_inline)) static inline void 
twim0_baud      (uint32_t cpuHz, uint32_t twiHz)
                {
                int32_t v = cpuHz/twiHz/2 - 5;
                TWI0.MBAUD = v >= 0 ? v : 0;
                }

