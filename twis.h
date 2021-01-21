#pragma once
/*------------------------------------------------------------------------------
    twis.h - Twi slave
------------------------------------------------------------------------------*/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;

typedef enum { ADDRESSED, MREAD, MWRITE, STOPPED } twis_irqstate_t;
typedef bool(*twis_callback_t)(twis_irqstate_t state, u8 status);

void twis_off           ();
void twis_write         (u8 v);
u8   twis_read          ();
u8   twis_lastAddress   ();
void twis_address2      (u8 addr2);
void twis_init          (u8 addr, twis_callback_t cb);
void twis_defaultPins   ();
void twis_altPins       ();

//pins

/* twi0 master
                mega0/DA    tiny0/1     tiny0/1 XX2 (8pin)
default SCL      PA3          PB0         PA2
default SDA      PA2          PB1         PA1
    alt SCL      PC3          PA2         --
    alt SDA      PC2          PA1         --
*/

#if defined(PORTMUX_TWISPIROUTEA) //mega0
#define TWIS_PULL_DEFAULT()     PORTA.PIN3CTRL |= 1<<3; PORTA.PIN2CTRL |= 1<<3
#define TWIS_PULL_ALT()         PORTC.PIN3CTRL |= 1<<3; PORTC.PIN2CTRL |= 1<<3
#define TWIS_PORTMUX_DEFAULT()  PORTMUX.TWISPIROUTEA &= ~PORTMUX_TWI0_gm;
#define TWIS_PORTMUX_ALT()      (PORTMUX.TWISPIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

#elif defined(TWIROUTEA) //avrda
#define TWIS_PULL_DEFAULT()     PORTA.PIN3CTRL |= 1<<3; PORTA.PIN2CTRL |= 1<<3
#define TWIS_PULL_ALT()         PORTC.PIN3CTRL |= 1<<3; PORTC.PIN2CTRL |= 1<<3
#define TWIS_PORTMUX_DEFAULT()  PORTMUX.TWISPIROUTEA &= ~PORTMUX_TWI0_gm;
#define TWIS_PORTMUX_ALT()      (PORTMUX.TWISPIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

#elif defined(PORTMUX_CTRLB) && defined(PORTMUX_TWI0_bm) //tiny0/1 w/alternate pins
#define TWIS_PULL_DEFAULT()     PORTB.PIN0CTRL |= 1<<3; PORTB.PIN1CTRL |= 1<<3
#define TWIS_PULL_ALT()         PORTA.PIN2CTRL |= 1<<3; PORTA.PIN1CTRL |= 1<<3
#define TWIS_PORTMUX_DEFAULT()  PORTMUX.CTRLB &= ~PORTMUX_TWI0_bm;
#define TWIS_PORTMUX_ALT()      PORTMUX.CTRLB |= PORTMUX_TWI0_bm;


#elif defined(PORTMUX_CTRLB) && !defined(PORTMUX_TWI0_bm) //tiny0/1 no alternate pins
#define TWIS_PORTMUX_DEFAULT()
#define TWIS_PORTMUX_ALT()
#define TWIS_PULL_DEFAULT() PORTA.PIN2CTRL |= 1<<4; PORTA.PIN1CTRL |= 1<<4
#define TWIS_PULL_ALT()     TWIS_PULL_DEFAULT()

#else
#error "Unknown portmux/pin settings for TWI0"
#endif
