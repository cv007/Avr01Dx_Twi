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

                //find a unique portmux define from the mcu header to group/select the pins for a series of avr's
                //then use its defines to specify the pin info

//mega0
                #if defined(PORTMUX_TWISPIROUTEA) 
                static twi_pins_t 
twi0_std_pins   = { &PORTA, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, 0 };
                static twi_pins_t 
twi0_alt_pins   = { &PORTC, 3, 2, &PORTMUX_TWISPIROUTEA, PORTMUX_TWI0_gm, PORTMUX_TWI0_ALT2_gc };
                
//avrda
                #elif defined(PORTMUX_TWIROUTEA)
                static twi_pins_t 
twi0_std_pins   = { &PORTA, 3, 2, &PORTMUX_TWIROUTEA, PORTMUX_TWI0_gm, 0 };
                    static twi_pins_t 
twi0_alt_pins   = { &PORTC, 3, 2, &PORTMUX_TWIROUTEA, PORTMUX_TWI0_gm, PORTMUX_TWI0_ALT2_gc };

//tiny0/1 w/alternate pins
                #elif defined(PORTMUX_CTRLB) && defined(PORTMUX_TWI0_bm)
                static twi_pins_t 
twi0_std_pins   = { &PORTB, 0, 1, &PORTMUX_CTRLB, PORTMUX_TWI0_bm, 0 };
                static twi_pins_t 
twi0_alt_pins   = { &PORTA, 2, 1, &PORTMUX_CTRLB, PORTMUX_TWI0_bm, PORTMUX_TWI0_bm };

//tiny0/1 no alternate pins
                #elif defined(PORTMUX_CTRLB) && !defined(PORTMUX_TWI0_bm)
                static twi_pins_t 
twi0_std_pins   = { &PORTB, 0, 1, 0, 0, 0 }; //pmux = 0 (portmux not used)
                static twi_pins_t 
twi0_alt_pins   = { &PORTB, 0, 1, 0, 0, 0 }; //use default pins
                //unknown
                #else
                #error "Unknown portmux/pin settings for TWI"
                #endif


                static inline __attribute(( always_inline )) void 
twi_pins_init   (twi_pins_t s)  //passing by value since is inline w/static data (compiler optimizes)
                {               //allows . member access instead of ->
                uint8_t 
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



