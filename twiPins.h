#pragma once
/*------------------------------------------------------------------------------
    twiPins.h - Twi pins
------------------------------------------------------------------------------*/
#include <avr/io.h>

/* twi0 master
                mega0/DA    tiny0/1     tiny0/1 XX2 (8pin)
default SCL      PA3          PB0         PA2
default SDA      PA2          PB1         PA1
    alt SCL      PC3          PA2         --
    alt SDA      PC2          PA1         --
*/

#if defined(PORTMUX_TWISPIROUTEA) //mega0
#define TWI_PULL_DEFAULT()      PORTA.PIN3CTRL |= 1<<3; PORTA.PIN2CTRL |= 1<<3
#define TWI_PULL_ALT()          PORTC.PIN3CTRL |= 1<<3; PORTC.PIN2CTRL |= 1<<3
#define TWI_PORTMUX_DEFAULT()   PORTMUX.TWISPIROUTEA &= ~PORTMUX_TWI0_gm;
#define TWI_PORTMUX_ALT()       PORTMUX.TWISPIROUTEA = (PORTMUX.TWISPIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

#elif defined(TWIROUTEA) //avrda
#define TWI_PULL_DEFAULT()      PORTA.PIN3CTRL |= 1<<3; PORTA.PIN2CTRL |= 1<<3
#define TWI_PULL_ALT()          PORTC.PIN3CTRL |= 1<<3; PORTC.PIN2CTRL |= 1<<3
#define TWI_PORTMUX_DEFAULT()   PORTMUX.TWIROUTEA &= ~PORTMUX_TWI0_gm;
#define TWI_PORTMUX_ALT()       PORTMUX.TWIROUTEA = (PORTMUX.TWIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

#elif defined(PORTMUX_CTRLB) && defined(PORTMUX_TWI0_bm) //tiny0/1 w/alternate pins
#define TWI_PULL_DEFAULT()      PORTB.PIN0CTRL |= 1<<3; PORTB.PIN1CTRL |= 1<<3
#define TWI_PULL_ALT()          PORTA.PIN2CTRL |= 1<<3; PORTA.PIN1CTRL |= 1<<3
#define TWI_PORTMUX_DEFAULT()   PORTMUX.CTRLB &= ~PORTMUX_TWI0_bm;
#define TWI_PORTMUX_ALT()       PORTMUX.CTRLB |= PORTMUX_TWI0_bm;


#elif defined(PORTMUX_CTRLB) && !defined(PORTMUX_TWI0_bm) //tiny0/1 no alternate pins
#define TWI_PORTMUX_DEFAULT()
#define TWI_PORTMUX_ALT()
#define TWI_PULL_DEFAULT()      PORTA.PIN2CTRL |= 1<<4; PORTA.PIN1CTRL |= 1<<4
#define TWI_PULL_ALT()          TWIS_PULL_DEFAULT()

#else
#error "Unknown portmux/pin settings for TWI0"
#endif



