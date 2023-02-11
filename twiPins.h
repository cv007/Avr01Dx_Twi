#pragma once
#include "MyAvr.h"

/*------------------------------------------------------------------------------
    twiPins.h - Twi pins

    twi0 master/slave - (not dual mode)

                mega0/DA    tiny0/1     tiny0/1 XX2 (8pin)
default SCL      PA3          PB0         PA2
default SDA      PA2          PB1         PA1
    alt SCL      PC3          PA2         --
    alt SDA      PC2          PA1         --

    add new mcu's to this pin info table as needed
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    uncomment the pair of pins that suits your mcu (instead of using a series
    of ifdef's which get harder to manage as more mcu's are added)

    add new pairs as needed (can also add twi1_ pins if needed)

    portmux register names are not all the same, and the twi 
    bitfields in the portmux register also can be different
    so will have to look these names up when creating a new
    pair of pins

    you can also pass a struct directly without these premade structs if wanted-
        twi_pins_init( (twi_pins_t){ &PORTA, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, 0 } );
------------------------------------------------------------------------------*/
                typedef struct {
                    PORT_t* port;               // &PORTn
                    uint8_t pinSCL;             // 0-7
                    uint8_t pinSCA;             // 0-7
                    volatile uint8_t* pmux;     // &PORTMUX.twi_register
                    uint8_t pmux_clrbm;         // bitmask values to clear/set the appropriate twi bitfields in portmux to select a set of pins
                    uint8_t pmux_setbm;         //  the clrbm (inverted) will be used to reset bitfield to default, the setbm will set the desired value
                } 
const 
twi_pins_t;

//------------------------------
// mega0
//------------------------------
//                 static twi_pins_t 
// twi0_std_pins   = { &PORTA, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, 0 };
//                 static twi_pins_t 
// twi0_alt_pins   = { &PORTC, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, PORTMUX_TWI0_ALT2_gc };

//------------------------------
// avrda
//------------------------------
//                 static twi_pins_t 
// twi0_std_pins   = { &PORTA, 3, 2, &PORTMUX_TWIROUTEA, PORTMUX_TWI0_gm, 0 };
//                     static twi_pins_t 
// twi0_alt_pins   = { &PORTC, 3, 2, &PORTMUX_TWIROUTEA, PORTMUX_TWI0_gm, PORTMUX_TWI0_ALT2_gc };

//------------------------------
// tiny0/1 w/alternate pins
//------------------------------
                static twi_pins_t 
twi0_std_pins   = { &PORTB, 0, 1, &PORTMUX_CTRLB, PORTMUX_TWI0_bm, 0 };
                static twi_pins_t 
twi0_alt_pins   = { &PORTA, 2, 1, &PORTMUX_CTRLB, PORTMUX_TWI0_bm, PORTMUX_TWI0_bm };

//------------------------------
// tiny0/1 no alternate pins
//------------------------------
//                 static twi_pins_t 
// twi0_std_pins   = { &PORTB, 0, 1, 0, 0, 0 }; //pmux = 0 (portmux not used)
//                 static twi_pins_t 
// twi0_alt_pins   = { &PORTB, 0, 1, 0, 0, 0 }; //use default pins



                __attribute(( always_inline )) static inline void 
twi_pins_init   (twi_pins_t s)  //passing by value since is inline w/static data (compiler optimizes)
                {               //allows . member access instead of ->, and can pass s by value (name vs &name)
                uint8_t         //and also allows passing a struct created as an argument (see notes above)
                    scl = s.pinSCL & 7,         //extract all values for easier use/reading
                    sca = s.pinSCA & 7, 
                    clrbm = ~s.pmux_clrbm,      //inverted for bitand use
                    setbm = s.pmux_setbm;
                volatile uint8_t 
                    *pinctrl = &s.port->PIN0CTRL, 
                    *pmux = s.pmux;
                //enable pullups and set portmux as needed (some have no alt pins, so no twi portmux)
                pinctrl[scl] = PORT_PULLUPEN_bm; //assignment, will set all other bits to 0
                pinctrl[sca] = PORT_PULLUPEN_bm; // if need invert or isc bits for some reason, change to |=
                if( pmux ) *pmux = (*pmux & clrbm) | setbm; //compiler will optimize if bitfield is a single bit
                }



