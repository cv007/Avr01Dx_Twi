/*------------------------------------------------------------------------------
    twis.c - Twi slave
------------------------------------------------------------------------------*/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "twis.h"
#include "twiPins.h"

    //============
    //  private:
    //============


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
static void     nackComplete    ()      { TWI0.SCTRLB = 6; } //COMPTRANS, NACK
static void     ack             ()      { TWI0.SCTRLB = 3; } //RESPONSE, ACK
//v = a copy of SSTATUS (used in isr)
static bool     isError         (u8 v)  { return (v & 0x0C); }          //either- COLL, BUSERR
static bool     isDataRead      (u8 v)  { return (v & 0x82) == 0x82; }  //DIF, DIR(1=R)
static bool     isDataWrite     (u8 v)  { return (v & 0x82) == 0x80; }  //DIF, DIR(0=W)
static bool     isAddress       (u8 v)  { return (v & 0x41) == 0x41; }  //APIF, AP(1=addr)
static bool     isStop          (u8 v)  { return (v & 0x41) == 0x40; }  //APIF, AP(0=stop)
static bool     isRxNack        (u8 v)  { return (v & 0x10); }          //RXACK(0=ACK,1=NACK)


                    //callback function returns true if all is fine
            ISR     (TWI0_TWIS_vect) {
                        static bool is1stbyte; //so can ignore rxack on first master read
                        u8 s = status(); //get a copy, so are dealing with 1 point in time

                        // collision, buserror, or stop
                        if( isError(s) || isStop(s) ){
                            isrFuncCallback_(STOPPED, s);
                            return nackComplete();
                        }

                        //address
                        if( isAddress(s) ){
                            lastAddress_ = read()>>1;
                            is1stbyte = true;
                            return isrFuncCallback_(ADDRESSED, s) ? ack() : nackComplete();
                        }

                        //data, master read (slave data -> master)
                        if( isDataRead(s) ){
                            if( ! is1stbyte && isRxNack(s) ) return nackComplete();
                            is1stbyte = false;
                            return isrFuncCallback_(MREAD, s) ? ack() : nackComplete();
                        }

                        //data, master write (master data -> slave)
                        if( isDataWrite(s) ){
                            return isrFuncCallback_(MWRITE,s) ? ack() : nackComplete();
                        }
                    }

    //============
    // public:
    //============

void twis_defaultPins   ()      { TWI_PULL_DEFAULT(); TWI_PORTMUX_DEFAULT(); }
void twis_altPins       ()      { TWI_PULL_ALT(); TWI_PORTMUX_ALT(); }
void twis_off           ()      { irqAllOff(); off(); clearFlags(); }
void twis_write         (u8 v)  { write(v); }
u8   twis_read          ()      { return read(); }
u8   twis_lastAddress   ()      { return lastAddress_; }    //last address we responded to
void twis_address2      (u8 v)  { address2(v); }            //2nd address

void twis_init          (u8 addr, twis_callback_t cb) {
                            if( ! cb ) return;              //assume everything other than 0 is valid
                            twis_off();                     //also clears flags
                            isrFuncCallback_ = cb;
                            address1( addr );
                            irqAllOn();
                            sei();
                            on();
                        }


