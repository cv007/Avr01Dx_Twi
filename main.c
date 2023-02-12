#include "MyAvr.h"

#include "twis0.h"
#include "twim0.h"

/*------------------------------------------------------------------------------
    demonstrate twi0 usage, both master and slave communicating on same device
    tested with an ATtiny416 Xplained Nano (led is PB5, inverted)
------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------
    Blinker I2C device

        slave address 0x51

        our slave device has a register at address 0x08 named onTime,
            and an address of 0x09 named offTime
        the onTime value will determine the led blink on time-
            0 = off
            1 = 10ms on
            255 = 2550ms on
        the offTime value will determine the led off time-
            0 = off
            1 = 10ms off
            255 = 2550ms off

        register address will auto increment after a write or read (if write
            was not a register address write), will roll over from end to beginning

        to write a new value to onTime or offTime
            write 0x08 or 0x09, write new value
        to read the current value of onTime or offTime
            write 0x08 or 0x09, read value
        to write a new value to onTime and offTime
            write 0x08, write new onTime value, write new offTime value
        to read the current value of onTime and offTime
            write 0x08, read onTime value, read offTime value

------------------------------------------------------------------------------*/
                struct blinker_s {
                    //registers
                    u8 unused_[8];
                    u8 onTime;          //[8], MS x10
                    u8 offTime;         //[9], MS x10
                    //cannot read/write below registers via twi
                    const u8 myAddress; //slave address
                    bool isFirstWr;     //is a register address write
                    u8* regPtr;         //current register address (pointer)
                } blinker = { {0}, 0, 0, 0x51, 0, false };

                bool
twis0Callback   (twis_irqstate_t state, u8 statusReg)
                {
                //keep regPtr inside the range of registers that can write/read
                if( blinker.regPtr > &blinker.offTime ||
                    blinker.regPtr < &blinker.unused_[0] ) blinker.regPtr = &blinker.unused_[0];
                bool ret = true; //assume ok to continue transaction

                switch( state ) {
                    //check address here, could be general call (0) or maybe we
                    //have a second address or address mask
                    case TWIS_ADDRESSED:
                        if( twis0_lastAddress() != blinker.myAddress ) ret = false; //for us?
                        else blinker.isFirstWr = true; //yes, expect a register address write
                        break;
                    case TWIS_MREAD:
                        twis0_write( *blinker.regPtr++ );
                        break;
                    case TWIS_MWRITE:
                        {
                        u8 v = twis0_read();
                        //if first write, is a register address write
                        //regPtr will be validated in next isr
                        if( blinker.isFirstWr ){
                            blinker.isFirstWr = false;
                            blinker.regPtr = &blinker.unused_[v];
                            break;
                            }
                        //else is a register write
                        *blinker.regPtr++ = v;
                        }
                        break;
                    case TWIS_STOPPED:
                    case TWIS_ERROR:
                        ret = false;
                        break;

                    }
                return ret;
                }

/*------------------------------------------------------------------------------
    led - PB5 (inverted) in this case
------------------------------------------------------------------------------*/
                typedef struct { PORT_t* port; u8 pin; bool invert; }
const
pin_t;
                static pin_t            //set as needed for your board
led             = { &PORTB, 5, true };  //PB5, low is on

                static void
pinSet          (pin_t p, bool on)
                {
                u8 pinbm = 1<<(p.pin & 7);
                p.port->DIRSET = pinbm;
                if( p.invert ) (&p.port->PIN0CTRL)[p.pin] |= 0x80;
                on ? (p.port->OUTSET = pinbm) : (p.port->OUTCLR = pinbm);
                }


/*------------------------------------------------------------------------------
    wait - uses _delay_ms (so we can have a simple callable delay with
           variable runtime values)
------------------------------------------------------------------------------*/
                static void
waitMS          (u16 ms){ while( ms-- ) _delay_ms(1); }



/*------------------------------------------------------------------------------
    twi0 master communications to slave device
    assume we have no access to the blinker struct above (which we would not
    normally have access to), so create any needed values (enum works good)
------------------------------------------------------------------------------*/
                //enums for Blinker device
                enum { BLINKER_SLAVE_ADDRESS = 0x51,            //address
                       BLINKER_ONTIME = 8, BLINKER_OFFTIME };   //register addresses

                //master to slave (blinker)
                static bool
blinkerWrite    (u8 reg, const u8* v, u8 vlen)
                {
                twim0_baud( F_CPU, 100000ul );
                twim0_on( BLINKER_SLAVE_ADDRESS );
                twim0_writeWrite( &reg, 1, v, vlen ); //write register address, write value(s)
                bool ret = twim0_waitUS( 3000 );
                twim0_off();
                return ret;
                }

                static bool
blinkerRead     (u8 reg, u8* v, u8 vlen)
                {
                twim0_baud( F_CPU, 100000ul );
                twim0_on( BLINKER_SLAVE_ADDRESS );
                twim0_writeRead( &reg, 1, v, vlen ); //write register address, read value(s)
                bool ret = twim0_waitUS( 3000 );
                twim0_off();
                return ret;
                }



/*------------------------------------------------------------------------------
    main
------------------------------------------------------------------------------*/
                int
main            ()
                {

                //setup blinker slave device
                twis0_on( blinker.myAddress, twis0Callback );
                sei();

                //blinker device has unused registers, will use register 0
                //to store a value so we can test reading the slave also
                u8 blinkN = 5; //blink N times in loop below
                blinkerWrite( 0, &blinkN, 1 ); //write 1 byte, 5-> register 0

                const u8 onOffTbl[] = {
                    2, 20,      //20ms on, 200ms off
                    100, 100    //1000ms on, 1000ms off
                    };
                u8 idx = 0;

                while(1) {

                    //keep idx inside table
                    if( idx >= sizeof(onOffTbl)/sizeof(onOffTbl[0]) ) idx = 0;

                    //write 2 values starting at ONTIME register (master->slave)
                    if( ! blinkerWrite(BLINKER_ONTIME, &onOffTbl[idx], 2) ){
                        //special case for bus recovery since we are using the same pins
                        //for master/slave in this example (not normal), so we have to get
                        //the slave to release the pins also
                        twis0_off();
                        twim0_busRecovery();    //failed (don't care why), so do recovery
                        _delay_ms( 1000 );      //delay
                        //and turn the slave back on
                        twis0_on( blinker.myAddress, twis0Callback );
                        continue;               //start over at while(1), try again
                        }
                    //next pair
                    idx += 2;

                    //get value from register 0 (should be same as blinkN initial value above)
                    //if any error, turn on led for 10 seconds
                    if( ! blinkerRead( 0, &blinkN, 1) ){
                        pinSet( led, 1 );
                        waitMS( 10000 );
                        pinSet( led, 0 );
                        continue; //start over
                    }

                    //blink N times (slave Blinker is doing this)
                    for( u8 i = 0; i < blinkN; i++ ){
                        pinSet( led, 1 );
                        waitMS( blinker.onTime * 10 );
                        pinSet( led, 0 );
                        waitMS( blinker.offTime * 10 );
                        }

                    //delay between blinks
                    waitMS( 2000 );

                    }

            }

