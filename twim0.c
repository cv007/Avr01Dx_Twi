//======================================================================
//  twim0.c - Twi0, master - avr mega0, tiny0/1, da
//======================================================================
#include "MyAvr.h"

#include "twim0.h"
#include "twiPins.h"

//==========
// private:
//==========

                static twim_callbackT   isrFuncCallback_;
                static const u8*        txbuf_;
                static const u8*        txbufEnd_;
                static const u8*        txbuf2_;
                static const u8*        txbuf2End_;
                static u8*              rxbuf_;
                static const u8*        rxbufEnd_;
                static volatile bool    lastResult_; //1=ok,0=fail

                //local enums

                //MCTRLB flush3|ack2|cmd1:0
                enum { ACK = 0, READ = 2, STOP = 3, NACK = 4,  FLUSH = 8 };
                //MSTATUS RIF7|WIF6|CLKHOLD5|RXACK4|ARBLOST3|BUSERR2|BUSSTATE1:0
                enum { RIF = 0x80, WIF = 0x40, CLKHOLD = 0x20, RXNACK = 0x10, ARBLOST = 0x8, BUSERR = 0x4 };
                enum { ALLFLAGS = RIF|WIF|CLKHOLD|ARBLOST|BUSERR };
                enum { ANYERR = ARBLOST|BUSERR }; //error bits
                enum { RIEN = RIF, WIEN = WIF, RWIEN = RIEN|WIEN }; //irq bits
                enum { RW = 1 }; //address bit0
                enum { UNKNOWN = 0, IDLE, OWNER, BUSBUSY, BUSMASK = 3 }; //bus state
                enum { READOK = RIF|CLKHOLD|OWNER, WRITEOK = WIF|CLKHOLD|OWNER };
                enum { ENABLE = 1 }; //on/off

                static void
on              () { TWI0.MCTRLA |= ENABLE; }
                static void
off             () { TWI0.MCTRLA = 0; }
                static void
irqAllOn        () { TWI0.MCTRLA |=  RWIEN; }
                static void
irqAllOff       () { TWI0.MCTRLA &= ~RWIEN; }
                static void
toStateIdle     () { TWI0.MSTATUS = ALLFLAGS|IDLE; } //clear flags, set to IDLE
                static void
ackActionACK    () { TWI0.MCTRLB = ACK; }
                static void
ACKread         () { TWI0.MCTRLB = READ; }
                static void
NACKstop        () { TWI0.MCTRLB = NACK|STOP; }
                static void
address         (u8 v) { off(); TWI0.MADDR = v<<1; } //off so no start produced
                static void
startRead       () { ackActionACK(); TWI0.MADDR |= RW; } //reuse existing address
                static void
startWrite      () { TWI0.MADDR &= ~RW; } //reuse existing address
                static void
write           (u8 v) { TWI0.MDATA = v; }
                static u8
read            () { return TWI0.MDATA; }
                static u8
status          () { return TWI0.MSTATUS; }
                static bool
isBusy          () { return TWI0.MCTRLA & RWIEN; }

                static void
startIrq        (bool wr) //start a read (wr=0) or write (wr=1), enable irq
                {
                wr ? startWrite() : startRead();
                lastResult_ = false;
                irqAllOn();
                }

                static void
finished        (bool tf) //for isr use, tf=true if success
                {
                lastResult_ = tf;
                //NACKstop works for write also (nack not done, harmless)
                NACKstop();
                irqAllOff(); //do before callback in case call back starts another xfer
                if( isrFuncCallback_ ) isrFuncCallback_();
                }

ISR             (TWI0_TWIM_vect)
                {
                u8 s = status();
                //error
                if( s & ANYERR ) return finished( false );
                //read
                if( s == READOK ){
                    *rxbuf_++ = read();
                    return rxbuf_ < rxbufEnd_ ? ACKread() : finished( true );
                    }
                //write
                if( s == WRITEOK ){
                    if( txbuf_ < txbufEnd_ ) return write( *txbuf_++ ); //more data
                    if( txbuf2_ < txbuf2End_ ) return write( *txbuf2_++ ); //more data
                    return rxbuf_ ? startRead() : finished( true ); //switch to read? or done
                    }
                //unknown, or a write nack
                finished( false );
                }

                static void 
initPins        (bool busRecovery) //false = no bus recovery, true = also do bus recovery
                { 
                uint8_t
                    scl = twi0_pins.MpinSCL & 7,        //extract all values for easier use/reading
                    sca = twi0_pins.MpinSCA & 7, 
                    clrbm = ~twi0_pins.pmux_clrbm,      //inverted for bitand use
                    setbm = twi0_pins.pmux_setbm;
                volatile uint8_t *pinctrl = &twi0_pins.Mport->PIN0CTRL; 
                volatile uint8_t *pmux = twi0_pins.pmux;

                off(); //turn off twi

                //enable pullups and set portmux as needed (some have no alt pins, so no twi portmux)
                pinctrl[scl] = PORT_PULLUPEN_bm; //assignment, will set all other bits to 0
                pinctrl[sca] = PORT_PULLUPEN_bm; // if need invert or isc bits for some reason, change to |=
                if( pmux ) *pmux = (*pmux & clrbm) | setbm; //compiler will optimize if bitfield is a single bit
                if( busRecovery == false ) return;

                //also do bus recovery

                uint8_t sclbm = 1<<(twi0_pins.MpinSCL & 7), scabm = 1<<(twi0_pins.MpinSCA & 7);
                PORT_t* pt = twi0_pins.Mport; 

                pt->OUTSET = sclbm;             //scl high
                pt->DIRSET = sclbm;             //scl output
                for( u8 i = 0; i < 19; i++ ){   //10 clocks (20 toggles, but leave low so 19)
                    pt->OUTTGL = sclbm;
                    _delay_us( 5 );             //5us half cycle = 100khz
                    }
                //produce a stop 
                pt->OUTCLR = scabm;             //sca low
                pt->DIRSET = scabm;             //sca output
                _delay_us( 30 );
                pt->DIRCLR = sclbm;             //scl back to input w/pullup
                _delay_us( 30 );
                pt->DIRCLR = scabm;             //sca back to input w/pullup
                }

//==========
// public:
//==========

                void
twim0_callback  (twim_callbackT cb) { isrFuncCallback_ = cb; } //optional, else use twim_waitUS
                void
twim0_off       () { off(); }
                void
twim0_on        (u8 addr) 
                { 
                initPins(false); //will also turn off twim (false=no bus recovery)
                address(addr); 
                toStateIdle(); 
                on(); 
                }
                bool
twim0_isBusy    () { return isBusy(); } //if irq on, is busy
                bool
twim0_resultOK  () { return lastResult_; }

                //write+read (or write only, or read only)
                void
twim0_writeRead (const u8* wbuf, u16 wn, u8* rbuf, u16 rn)
                {
                txbuf_ = wbuf; txbufEnd_ = &wbuf[wn];
                rxbuf_ = rbuf; rxbufEnd_ = &rbuf[rn];
                txbuf2_ = 0; txbuf2End_ = 0;
                startIrq( wn ); //if no write (wn==0), then will start a read irq
                }

                //write/write (such as a command, then a buffer)
                void
twim0_writeWrite(const u8* wbuf, u16 wn, const u8* wbuf2, u16 wn2)
                {
                txbuf_ = wbuf; txbufEnd_ = &wbuf[wn];
                txbuf2_ = wbuf2; txbuf2End_ = &wbuf2[wn2];
                rxbuf_ = 0; rxbufEnd_ = 0; //no read
                startIrq( 1 ); //write only
                }

                //write only alias
                void
twim0_write     (const u8* wbuf, u16 wn) { twim0_writeRead( wbuf, wn, 0, 0); }

                //read only alias
                void
twim0_read      (u8* rbuf, u16 rn) { twim0_writeRead( 0, 0, rbuf, rn); }

                //blocking wait with timeout
                //if false is returned, caller can check twim0_isBusy() to see
                //if was a timeout or an error (isBusy will be true if timeout)
                //caller then can do a bus recovery if wanted
                bool
twim0_waitUS    (u16 us)
                {
                while( _delay_us(1), --us && twim0_isBusy() ){}
                return twim0_resultOK(); //true = ok, false = error or timeout
                }

                //recover locked up bus
                //NOTE: if you are running the slave on the same pins for some reason
                //      (not normal), the slave will also need to be disabled so it
                //      releases its pins (which are the same pins)
                void 
twim0_busRecovery() { initPins(true); }
