                #pragma once

                //======================================================================
                //  pmux.h
                //======================================================================

                #include "MyAvr.h"

                //------------------------------
                // mega0
                //------------------------------
                #if defined PORTMUX_TWISPIROUTEA

                typedef enum { PMUX_TWI0_DEFAULT = 0, PMUX_TWI0_ALT1 = 1,PMUX_TWI0_ALT2 = 2 } pmux_twi_t;

                static inline void
pmux_twi        (pmux_twi_t e)
                {
                PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & ~(3<<4)) | (e<<4);
                }


                //------------------------------
                // tiny0/1 w/alternate pins
                //------------------------------
                #elif defined PORTMUX_CTRLB && defined PORTMUX_TWI0_bm

                typedef enum { PMUX_TWI0_DEFAULT = 0, PMUX_TWI0_ALT = 1 } pmux_twi_t;

                static inline void
pmux_twi        (pmux_twi_t e)
                {
                PORTMUX.CTRLB = (PORTMUX.CTRLB & ~(1<<4)) | (e<<4);
                }


                //------------------------------
                // tiny0/1 no alternate pins
                //------------------------------
                #elif defined PORTMUX_CTRLB

                //allow twiPins.h to populate twi_pin_t, but will be unused
                typedef enum { PMUX_TWI0_DEFAULT = 0 } pmux_twi0_t;


                //------------------------------
                // unknown avr
                //------------------------------
                #else
                #error "pmux.h - unknown avr in use, add code as needed"
                #endif
