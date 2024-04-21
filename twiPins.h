                #pragma once

                //======================================================================
                //  twiPins.h
                //======================================================================

                #include "MyAvr.h"
                #include "pmux.h"

                //inline keyword prevents compiler warning about unused functions
                //static keyword will allow compiler to optimize away unused functions

                //TWI0

                //use to define a set of pins and portmux value for twi
                typedef struct {
                    PORT_t* pt; //&PORTn
                    u8 scl_pn, sda_pn; //0-7
                    pmux_twi_t pmux; //PMUX_TWIn_DEFAULT[|ALT1|ALT2]
                    } const twi_pin_t;


                // bus recovery
                // twi peripheral needs to be off when called
                // used by master code
                static inline void
twi_bus_recovery(twi_pin_t p)
                {
                u8 scl_bm = 1<<p.scl_pn;
                u8 sda_bm = 1<<p.sda_pn;
                PORT_t* port = p.pt;
                port->OUTSET = scl_bm; //scl high
                port->DIRSET = scl_bm; //scl output
                for( u8 i = 0; i < 19; i++ ){ //19 toggles, so is left low after loop
                    port->OUTTGL = scl_bm;
                    _delay_us( 5 ); //5us half cycle = 100khz
                    }
                //now produce a stop
                port->OUTCLR = sda_bm; //sda low
                port->DIRSET = sda_bm; //sda output
                _delay_us( 10 );
                port->DIRCLR = scl_bm; //scl back to input w/pullup
                _delay_us( 10 );
                port->DIRCLR = sda_bm; //sca back to input w/pullup
                }

                static inline void
twi_init_pins   (twi_pin_t p)
                {
                (&p.pt->PIN0CTRL)[p.scl_pn] = PORT_PULLUPEN_bm; //scl pullup
                (&p.pt->PIN0CTRL)[p.sda_pn] = PORT_PULLUPEN_bm; //sda pullup
                p.pt->DIRCLR = (1<<p.scl_pn)|(1<<p.sda_pn); //input (for when twi is off)
                }


                //------------------------------
                // mega0
                //------------------------------
                #if defined PORTMUX_TWISPIROUTEA

                static twi_pin_t twi_PA32 = { &PORTA, 3, 2, PMUX_TWI0_DEFAULT };
                static twi_pin_t twi_PC32 = { &PORTC, 3, 2, PMUX_TWI0_ALT2 };
                //only used in dual mode
                static twi_pin_t twi_PF32 = { &PORTF, 3, 2, PMUX_TWI0_ALT1 };

                static inline void
twi0_init_PA32  ()
                {
                twi_init_pins(twi_PA32);
                pmux_twi( twi_PA32.pmux );
                }

                static inline void
twi0_recover_PA32() { twi_bus_recovery(twi_PA32); }

                static inline void
twi0_init_PC32  ()
                {
                twi_init_pins(twi_PC32);
                pmux_twi( twi_PC32.pmux );
                }

                static inline void
twi0_recover_PC32() { twi_bus_recovery(twi_PC32); }

                //dual mode

                static inline void
twi0_init_PA32_PC32()
                {
                twi_init_pins(twi_PA32);
                twi_init_pins(twi_PC32);
                pmux_twi( twi_PA32.pmux ); //default
                TWI0.DUALCTRL |= 3; //FM+ enable, dual enable
                }


                static inline void
twi0_init_PA32_PF32()
                {
                twi_init_pins(twi_PA32);
                twi_init_pins(twi_PF32);
                pmux_twi( twi_PF32.pmux ); //alt1
                TWI0.DUALCTRL |= 3; //FM+ enable, dual enable
                }

                static inline void
twi0_init_PC32_PF32()
                {
                twi_init_pins(twi_PC32);
                twi_init_pins(twi_PF32);
                pmux_twi( twi_PC32.pmux ); //alt2
                TWI0.DUALCTRL |= 3; //FM+ enable, dual enable
                }


                //------------------------------
                // tiny0/1 w/alternate pins
                //------------------------------
                #elif defined PORTMUX_CTRLB && defined PORTMUX_TWI0_bm

                static twi_pin_t twi_PB01 = { &PORTB, 0, 1, PMUX_TWI0_DEFAULT };
                static twi_pin_t twi_PA21 = { &PORTA, 2, 1, PMUX_TWI0_ALT };

                static inline void
twi0_init_PB01  ()
                {
                twi_init_pins( twi_PB01 );
                pmux_twi( twi_PB01.pmux );
                }

                static inline void
twi0_recover_PB01() { twi_bus_recovery( twi_PB01 ); }

                static inline void
twi0_init_PA21  ()
                {
                twi_init_pins( twi_PA21 );
                pmux_twi( twi_PA21.pmux );
                }

                static inline void
twi0_recover_PA21() { twi_bus_recovery( twi_PA21 ); }

                //------------------------------
                // tiny0/1/2 no alternate pins
                //------------------------------
                #else

                static twi_pin_t twi_PB01 = { &PORTB, 0, 1, PMUX_TWI0_DEFAULT };

                static inline void
twi0_init_PB01  ()
                {
                twi_init_pins(twi_PB01);
                //no portmux
                }

                static inline void
twi0_recover_PB01() { twi_bus_recovery(twi_PB01); }

                #endif



                /*

                create defines to alias the common function names to the
                specific functions that will init the pins, or do a bus
                recovery for the master

                examples

                //master on PB01
                #define twim0_init_pins()       twi0_init_PB01()
                #define twis0_init_pins()       //unused
                #define twim0_bus_recovery()    twi0_recover_PB01()

                //slave on PB01
                #define twim0_init_pins()       //unused
                #define twis0_init_pins()       twi0_init_PB01()
                #define twim0_bus_recovery()    //unused

                //master+slave in dual mode
                #define twim0_init_pins()       twi0_init_PA32_PC32()
                #define twis0_init_pins()       //unused
                #define twim0_bus_recovery()    twi0_recover_PA32() //master pins
                */


                //for the example app, we are using the same pins for master/slave
                //(so slave will not also init pins, our define set to do nothing)
                #define twim0_init_pins()       twi0_init_PB01()
                #define twis0_init_pins()       //unused
                //for master, set which pins to use to do bus recovery
                #define twim0_recover_pins()    twi0_recover_PB01()
