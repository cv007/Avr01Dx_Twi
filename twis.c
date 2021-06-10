//======================================================================
//   twis.c - Twi slave
///======================================================================
#include "MyAvr.h"

#include "twis.h"
#include "twiPins.h"

    //============
    //  private:
    //============

static volatile u8      lastAddress_; //could also skip this, and simply make
                                      //the callback do a read in the TWIS_ADDRESSED
                                      //state if it needs to know the address when
                                      //multiple addresses are in use, but will just
                                      //store it here so callback does not need to
static twis_callback_t  isrFuncCallback_;


static void address1        (u8 v) { TWI0.SADDR = (v<<1)|1; } //gencall enabled, so check address in callback
static void address2        (u8 v, bool nomask) { TWI0.SADDRMASK = (v<<1)|nomask; }
static void off             () { TWI0.SCTRLA &= ~1; }
static void on              () { TWI0.SCTRLA |= 1; }
static u8   read            () { return TWI0.SDATA; }
static void write           (u8 v) { TWI0.SDATA = v; }
static void irqAllOn        () { TWI0.SCTRLA |= 0xE0; }
static void irqAllOff       () { TWI0.SCTRLA &= ~0xE0; }
static u8   status          () { return TWI0.SSTATUS; }
static void clearFlags      () { TWI0.SSTATUS = 0xCC; }
static void nackComplete    () { TWI0.SCTRLB = 6; } //COMPTRANS, NACK
static void ack             () { TWI0.SCTRLB = 3; } //RESPONSE, ACK

//DIF:APIF:CLKHOLD:RXACK:COLL:BUSERR:DIR:AP
enum { DIF_DIRbm = 0x82, APIF_APbm = 0x41, RXNACKbm = 0x10, ERRbm = 0x0C,
       DIF_R = 0x82, DIF_W = 0x80, APIF_ADDRbm = 0x41, APIF_STOPbm = 0x40 };

                            //v = a copy of SSTATUS (used in isr)
static bool isDataRead      (u8 v) { return (v & DIF_DIRbm) == DIF_R; }         //DIF, DIR(1=R)
static bool isDataWrite     (u8 v) { return (v & DIF_DIRbm) == DIF_W; }         //DIF, DIR(0=W)
static bool isAddress       (u8 v) { return (v & APIF_APbm) == APIF_ADDRbm; }   //APIF, AP(1=addr)
static bool isStop          (u8 v) { return (v & APIF_APbm) == APIF_STOPbm; }   //APIF, AP(0=stop)
static bool isRxNack        (u8 v) { return (v & RXNACKbm); }                   //RXACK(0=ACK,1=NACK)
static bool isError         (u8 v) { return (v & ERRbm); }                      //COLL,BUSERR




                            //callback function returns true if want to proceed
ISR                         (TWI0_TWIS_vect)
                            {
                            static bool is1st;      //so can ignore rxack on first master read
                            u8 s = status();        //get a copy of status
                            twis_irqstate_t state = isError(s)     ? TWIS_ERROR : //do first
                                                    isStop(s)      ? TWIS_STOPPED :
                                                    isAddress(s)   ? TWIS_ADDRESSED :
                                                    isDataRead(s)  ? TWIS_MREAD :
                                                    isDataWrite(s) ? TWIS_MWRITE : TWIS_ERROR;
                            bool nacked = isRxNack(s);
                            bool done = false; //assume not done

                            if( state == TWIS_ADDRESSED ) {
                                lastAddress_ = read()>>1;
                                is1st = true;
                                }
                            else if( state == TWIS_MREAD ) {
                                if( is1st ) is1st = false; else done = nacked;
                                }
                            else if( state != TWIS_MWRITE ) done = true; //error or stopped

                            if( false == isrFuncCallback_(state, s) ) done = true;
                            if( done ) nackComplete(); else ack();
                            }

    //============
    // public:
    //============

void    twis_defaultPins    () { TWI_PULL_DEFAULT(); TWI_PORTMUX_DEFAULT(); }
void    twis_altPins        () { TWI_PULL_ALT(); TWI_PORTMUX_ALT(); }
void    twis_off            () { irqAllOff(); off(); clearFlags(); }
void    twis_write          (u8 v) { write(v); }
u8      twis_read           () { return read(); }
u8      twis_lastAddress    () { return lastAddress_; }    //last address we responded to
void    twis_address2       (u8 v) { address2(v, true); }      //2nd address
void    twis_addressMask    (u8 v) { address2(v, false); }     //address mask (no 2nd address)

void    twis_init           (u8 addr, twis_callback_t cb)
                            {
                            if( ! cb ) return;          //assume everything other than 0 is valid
                            twis_off();                 //also clears flags
                            isrFuncCallback_ = cb;
                            address1( addr );
                            irqAllOn();
                            on();
                            }


