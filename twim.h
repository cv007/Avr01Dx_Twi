#pragma once
//======================================================================
//  twim.h - Twi0, master - avr mega0, tiny0/1, da
//======================================================================
#include <avr/io.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;

typedef void (*twim_callbackT)(void);

void twim_address       (u8 address);
void twim_off           ();
void twim_on            ();
bool twim_isBusy        ();
bool twim_lastResultOK  ();
void twim_callback      (twim_callbackT myFunction);
void twim_writeRead     (const u8* write_buffer, u16 write_length,
                         u8* read_buffer, u16 read_length);
void twim_writeWrite    (const u8* write_buffer, u16 write_length,
                         const u8* write_buffer2, u16 write_length2);
void twim_write         (const u8* write_buffer, u16 write_length);
void twim_read          (u8* read_buffer, u16 read_length);
bool twim_wait          (u16 us);
void twim_defaultPins   ();
void twim_altPins       ();

//baud
inline __attribute((always_inline)) static
void twim_baud   (uint32_t cpuHz, uint32_t twiHz){
                            int32_t v = cpuHz/twiHz/2.0 + 0.99 - 5;
                            TWI0.MBAUD = v >= 0 ? v : 0;
                        }
//pins

/* twi0 master
                mega0/DA    tiny0/1     tiny0/1 XX2 (8pin)
default SCL      PA3          PB0         PA2
default SDA      PA2          PB1         PA1
    alt SCL      PC3          PA2         --
    alt SDA      PC2          PA1         --
*/

#if defined(PORTMUX_TWISPIROUTEA) //mega0
#define TWIM_PULL_DEFAULT() PORTA.PIN3CTRL |= 1<<3; PORTA.PIN2CTRL |= 1<<3
#define TWIM_PULL_ALT()     PORTC.PIN3CTRL |= 1<<3; PORTC.PIN2CTRL |= 1<<3
#define PORTMUX_DEFAULT()   PORTMUX.TWISPIROUTEA &= ~PORTMUX_TWI0_gm;
#define PORTMUX_ALT()       (PORTMUX.TWISPIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

#elif defined(TWIROUTEA) //avrda
#define TWIM_PULL_DEFAULT() PORTA.PIN3CTRL |= 1<<3; PORTA.PIN2CTRL |= 1<<3
#define TWIM_PULL_ALT()     PORTC.PIN3CTRL |= 1<<3; PORTC.PIN2CTRL |= 1<<3
#define PORTMUX_DEFAULT()   PORTMUX.TWIROUTEA &= ~PORTMUX_TWI0_gm;
#define PORTMUX_ALT()       (PORTMUX.TWIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

#elif defined(PORTMUX_CTRLB) && defined(PORTMUX_TWI0_bm) //tiny0/1 w/alternate pins
#define TWIM_PULL_DEFAULT() PORTB.PIN0CTRL |= 1<<3; PORTB.PIN1CTRL |= 1<<3
#define TWIM_PULL_ALT()     PORTA.PIN2CTRL |= 1<<3; PORTA.PIN1CTRL |= 1<<3
#define PORTMUX_DEFAULT()   PORTMUX.CTRLB &= ~PORTMUX_TWI0_bm;
#define PORTMUX_ALT()       PORTMUX.CTRLB |= PORTMUX_TWI0_bm;


#elif defined(PORTMUX_CTRLB) && !defined(PORTMUX_TWI0_bm) //tiny0/1 no alternate pins
#define PORTMUX_DEFAULT()
#define PORTMUX_ALT()
#define TWIM_PULL_DEFAULT() PORTA.PIN2CTRL |= 1<<4; PORTA.PIN1CTRL |= 1<<4
#define TWIM_PULL_ALT()     TWIM_PULL_DEFAULT()

#else
#error "Unknown portmux/pin settings for TWI0"
#endif
