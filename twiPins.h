#pragma once
#include "MyAvr.h"

/*------------------------------------------------------------------------------
    twiPins.h - Twi pins

    twi0 master/slave - basic/limited table for twim0 (not dual mode)

                mega0/DA    tiny0/1     tiny0/1 XX2 (8pin)
default SCL      PA3          PB0         PA2
default SDA      PA2          PB1         PA1
    alt SCL      PC3          PA2         --
    alt SDA      PC2          PA1         --

    add new mcu's to this pin info table as needed
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    uncomment the set of pins that suits your mcu (instead of using a 
    series of ifdef's which get harder to manage as more mcu's are added)

    add more as needed (can also add twi1_ pins if needed)

    portmux register names are not all the same, and the twi 
    bitfields in the portmux register also can be different
    so will have to look these names up when creating a new
    pair of pins

    master/slave pins are the same for many mcu's, but is split so one can
    use dual mode for those mcu's which are able to use a different set of 
    pins for master/slave
------------------------------------------------------------------------------*/
                typedef struct {
                    PORT_t* Mport;              // master port- &PORTn
                    uint8_t MpinSCL;            // 0-7
                    uint8_t MpinSCA;            // 0-7
                    PORT_t* Sport;              // slave port- &PORTn
                    uint8_t SpinSCL;            // 0-7
                    uint8_t SpinSCA;            // 0-7
                    volatile uint8_t* pmux;     // &PORTMUX.twi_register
                    uint8_t pmux_clrbm;         // bitmask values to clear/set the appropriate twi bitfields in portmux to select a set of pins
                    uint8_t pmux_setbm;         //  the clrbm (inverted) will be used to reset bitfield to default, the setbm will set the desired value
                } 
const 
twiPins_t;

//------------------------------
// mega0
//------------------------------
//                 static twiPins_t //std
// twi0_pins       = { &PORTA, 3, 2, &PORTA, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, 0 };
//                 static twiPins_t //alt
// twi0_pins       = { &PORTC, 3, 2,  &PORTC, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, PORTMUX_TWI0_ALT2_gc };

//------------------------------
// avrda
//------------------------------
//                 static twiPins_t //std
// twi0_pins       = { &PORTA, 3, 2, &PORTA, 3, 2, &PORTMUX_TWIROUTEA, PORTMUX_TWI0_gm, 0 };
//                 static twiPins_t //alt
// twi0_pins       = { &PORTC, 3, 2, &PORTC, 3, 2, &PORTMUX_TWIROUTEA, PORTMUX_TWI0_gm, PORTMUX_TWI0_ALT2_gc };

//------------------------------
// tiny0/1 w/alternate pins
//------------------------------
                static twiPins_t //std
twi0_pins       = { &PORTB, 0, 1, &PORTB, 0, 1, &PORTMUX_CTRLB, PORTMUX_TWI0_bm, 0 };
//                 static twiPins_t //alt
// twi0_pins       = { &PORTA, 2, 1, &PORTA, 2, 1, &PORTMUX_CTRLB, PORTMUX_TWI0_bm, PORTMUX_TWI0_bm };

//------------------------------
// tiny0/1 no alternate pins
//------------------------------
//                 static twiPins_t //std only
// twi0_std_pins   = { &PORTB, 0, 1, &PORTB, 0, 1, 0, 0, 0 }; //pmux = 0 (no twi portmux)




