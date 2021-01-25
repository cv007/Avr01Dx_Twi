/*------------------------------------------------------------------------------
    twis.c - Twi slave
------------------------------------------------------------------------------*/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "twis.h"

    //============
    //  private:
    //============

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
#define TWIS_PORTMUX_DEFAULT()  PORTMUX.TWIROUTEA &= ~PORTMUX_TWI0_gm;
#define TWIS_PORTMUX_ALT()      (PORTMUX.TWIROUTEA & ~PORTMUX_TWI0_gm) | PORTMUX_TWI0_ALT2_gc;

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


static volatile u8      lastAddress_;
static twis_callback_t  isrFuncCallback_;

static void     address1        (u8 v)  { TWI0.SADDR = v<<1; }
static void     address2        (u8 v)  { TWI0.SADDRMASK = (v<<1)|1; }
static void     off             ()      { TWI0.SCTRLA &= ~1; }
static void     on              ()      { TWI0.SCTRLA |= 1; }
static u8       read            ()      { return TWI0.SDATA; }
static void     write           (u8 v)  { TWI0.SDATA = v; }
static void     irqAllOn        ()      { TWI0.SCTRLA |= 0xE0; }
static void     irqAllOff       ()      { TWI0.SCTRLA &= ~0xE0; }
static u8       status          ()      { return TWI0.SSTATUS; }
static void     clearFlags      ()      { TWI0.SSTATUS = 0xCC; }
static void     complete        ()      { TWI0.SCTRLB = 2; }
static void     ack             ()      { TWI0.SCTRLB = 3; } //RESPONSE, ACK
static void     nack            ()      { TWI0.SCTRLB = 7; } //RESPONSE, NACK
//v = a copy of SSTATUS (used in isr)
static bool     isError         (u8 v)  { return (v & 0x0C); } //either- COLL, BUSERR
static bool     isDataRead      (u8 v)  { return (v & 0x82) == 0x82; } //DIF, DIR(1=R)
static bool     isDataWrite     (u8 v)  { return (v & 0x82) == 0x80; } //DIF, DIR(0=W)
static bool     isAddress       (u8 v)  { return (v & 0x41) == 0x41; } //APIF, AP(1=addr)
static bool     isStop          (u8 v)  { return (v & 0x41) == 0x40; } //APIF, AP(0=stop)
static bool     isRxNack        (u8 v)  { return (v & 0x10); } //RXACK(0=ACK,1=NACK)


                    //callback function returns true if all is fine
            ISR     (TWI0_TWIS_vect) {
                        static bool is1stbyte; //so can ignore rxack on first master read
                        u8 s = status(); //get a copy, so are dealing with 1 point in time

                        // collision, buserror, or stop
                        if( isError(s) || isStop(s) ){
                            isrFuncCallback_(STOPPED, s);
                            return complete();
                        }

                        //address
                        if( isAddress(s) ){
                            lastAddress_ = read()>>1;
                            is1stbyte = true;
                            return isrFuncCallback_(ADDRESSED, s) ? ack() : nack();
                        }

                        //data, master read (slave data -> master)
                        if( isDataRead(s) ){
                            if( ! is1stbyte && isRxNack(s) ) return complete();
                            is1stbyte = false;
                            return isrFuncCallback_(MREAD, s) ? ack() : complete();
                        }

                        //data, master write (master data -> slave)
                        if( isDataWrite(s) ){
                            return isrFuncCallback_(MWRITE,s) ? ack() : nack();
                        }
                    }

    //============
    // public:
    //============

void twis_defaultPins   ()      { TWIS_PULL_DEFAULT(); TWIS_PORTMUX_DEFAULT(); }
void twis_altPins       ()      { TWIS_PULL_ALT(); TWIS_PORTMUX_ALT(); }
void twis_off           ()      { irqAllOff(); off(); clearFlags(); }
void twis_write         (u8 v)  { write(v); }
u8   twis_read          ()      { return read(); }
u8   twis_lastAddress   ()      { return lastAddress_; } //last address we responded to
void twis_address2      (u8 v)  { address2(v); } //2nd address

void twis_init          (u8 addr, twis_callback_t cb) {
                            if( ! cb ) return; //assume everything other than 0 is valid
                            twis_off(); //also clears flags
                            isrFuncCallback_ = cb;
                            address1( addr );
                            irqAllOn();
                            sei();
                            on();
                        }


