                #pragma once

                //======================================================================
                //  twiPins.h
                //======================================================================

                #include "MyAvr.h"

                //inline keyword prevents compiler warning about unused functions
                //static keyword will allow compiler to optimize away unused functions

                //TWI0


                // bus recovery
                // twi peripheral needs to be off when called
                // used by master code

                static inline void
bus_recovery    (PORT_t* const port, u8 const scl_pin, u8 const sda_pin)
                {
                u8 scl_bm = 1<<scl_pin;
                u8 sda_bm = 1<<sda_pin;
                port->OUTSET = scl_bm; //scl high
                port->DIRSET = scl_bm; //scl output
                for( u8 i = 0; i < 19; i++ ){ //19 toggles, so is left low after loop
                    port->OUTTGL = scl_bm;
                    _delay_us( 5 ); //5us half cycle = 100khz
                    }
                //now produce a stop
                port->OUTCLR = sda_bm; //sda low
                port->DIRSET = sda_bm; //sda output
                _delay_us( 30 );
                port->DIRCLR = scl_bm; //scl back to input w/pullup
                _delay_us( 30 );
                port->DIRCLR = sda_bm; //sca back to input w/pullup
                }


                //------------------------------
                // mega0
                //------------------------------
                #if defined PORTMUX_TWISPIROUTEA

                static inline void
init_PA32   	()
                {
                PORTA.PIN3CTRL = PORT_PULLUPEN_bm; //scl pullup on
                PORTA.PIN2CTRL = PORT_PULLUPEN_bm; //sda pullup on
                PORTA.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & PORTMUX_TWI0_gm);
                }

                static inline void 
recover_PA32    () { bus_recovery(&PORTA,3,2; }

                static inline void
init_PC32   	()
                {
                PORTC.PIN3CTRL = PORT_PULLUPEN_bm; //scl pullup on
                PORTC.PIN2CTRL = PORT_PULLUPEN_bm; //sda pullup on
                PORTC.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;
                }

                static inline void 
recover_PC32    () { bus_recovery(&PORTC,3,2); }

                //dual mode

                static inline void
init_PA32_PC32	()
                {
                PORTA.PIN3CTRL = PORT_PULLUPEN_bm; //master scl pullup on
                PORTA.PIN2CTRL = PORT_PULLUPEN_bm; //master sda pullup on
                PORTC.PIN3CTRL = PORT_PULLUPEN_bm; //slave scl pullup on
                PORTC.PIN2CTRL = PORT_PULLUPEN_bm; //slave sda pullup on
                PORTA.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTC.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & PORTMUX_TWI0_gm);
                TWI0.DUALCTRL |= 1; //dual enable
                }


                static inline void
init_PA32_PF32	()
                {
                PORTA.PIN3CTRL = PORT_PULLUPEN_bm; //master scl pullup on
                PORTA.PIN2CTRL = PORT_PULLUPEN_bm; //master sda pullup on
                PORTF.PIN3CTRL = PORT_PULLUPEN_bm; //slave scl pullup on
                PORTF.PIN2CTRL = PORT_PULLUPEN_bm; //slave sda pullup on
                PORTA.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTF.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT1_gc;
                TWI0.DUALCTRL |= 3; //FM+ enable, dual enable
                }

                static inline void
init_PC32_PF32	()
                {
                PORTC.PIN3CTRL = PORT_PULLUPEN_bm; //master scl pullup on
                PORTC.PIN2CTRL = PORT_PULLUPEN_bm; //master sda pullup on
                PORTF.PIN3CTRL = PORT_PULLUPEN_bm; //slave scl pullup on
                PORTF.PIN2CTRL = PORT_PULLUPEN_bm; //slave sda pullup on
                PORTC.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTF.DIRCLR = PIN3_bm|PIN2_bm; //input (for when twi is off)
                PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;
                TWI0.DUALCTRL |= 3; //FM+ enable, dual enable
                }


                //------------------------------
                // tiny0/1 w/alternate pins
                //------------------------------
                #elif defined PORTMUX_CTRLB

                static inline void
init_PB01   	()
                {
                PORTB.PIN0CTRL = PORT_PULLUPEN_bm; //scl pullup on
                PORTB.PIN1CTRL = PORT_PULLUPEN_bm; //sda pullup on
                PORTB.DIRCLR = PIN0_bm|PIN1_bm; //input (for when twi is off)
                PORTMUX.CTRLB = (PORTMUX.CTRLB & PORTMUX_TWI0_bm);
                }

                static inline void 
recover_PB01    () { bus_recovery(&PORTB,0,1); }

                static inline void
init_PA21   	()
                {
                PORTA.PIN2CTRL = PORT_PULLUPEN_bm; //scl pullup on
                PORTA.PIN1CTRL = PORT_PULLUPEN_bm; //sda pullup on
                PORTA.DIRCLR = PIN2_bm|PIN1_bm; //input (for when twi is off)
                PORTMUX.CTRLB |= PORTMUX_TWI0_bm;
                }

                static inline void 
recover_PA21    () { bus_recovery(&PORTA,2,1); }

                //------------------------------
                // tiny0/1 no alternate pins
                //------------------------------
                #else

                static inline void
init_PB01   	()
                {
                PORTB.PIN0CTRL = PORT_PULLUPEN_bm; //scl pullup on
                PORTB.PIN1CTRL = PORT_PULLUPEN_bm; //sda pullup on
                PORTB.DIRCLR = PIN0_bm|PIN1_bm; //input (for when twi is off)
                }

                static inline void 
recover_PB01    () { bus_recovery(&PORTB,0,1); }

                #endif

                




                /*

                create defines to alias the common function names to the
                specific functions that will init the pins, or do a bus
                recovery for the master
                
                examples

                //master on PB01
                #define twim0_init_pins()       init_PB01()
                #define twis0_init_pins()       //unused  
                #define twim0_bus_recovery()    recover_PB01()       

                //slave on PB01
                #define twim0_init_pins()       //unused
                #define twis0_init_pins()       init_PB01()
                #define twim0_bus_recovery()    //unused 

                //master+slave in dual mode
                #define twim0_init_pins()       init_PA32_PC32()
                #define twis0_init_pins()       //unused
                #define twim0_bus_recovery()    recover_PA32() //master pins
                */


                //for the example app, we are using the same pins for master/slave
                #define twim0_init_pins()       init_PB01()
                #define twis0_init_pins()
                #define twim0_recover_pins()    recover_PB01()