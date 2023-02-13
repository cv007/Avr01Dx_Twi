//======================================================================
//  main.c
//
//  demonstrate twi0 usage, both master and slave communicating on same device
//  tested with an ATtiny416 Xplained Nano (led is PB5, inverted)
//======================================================================
#include "MyAvr.h"
#include "twis0.h"
#include "twim0.h"


/*------------------------------------------------------------------------------
    wait - uses _delay_ms (so we can have a simple callable delay with
           variable runtime values)
------------------------------------------------------------------------------*/
                static void
waitMS          (u16 ms){ while( ms-- ) _delay_ms(1); }


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

                static void
ledOnMS         (u16 ms){ pinSet( led, 1 ); waitMS( ms ); }
                static void
ledOffMS        (u16 ms){ pinSet( led, 0 ); waitMS( ms );  }


/*------------------------------------------------------------------------------
    Blinker I2C device - example i2c device on this mcu

        twi0 slave address 0x51

        our slave device has registers from address 0x00 to 0x07 named ram,
            which can be used to store byte values
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
                    u8 ram[8];
                    u8 onTime;          //[8], MS x10
                    u8 offTime;         //[9], MS x10
                    //cannot read/write below registers via twi
                    const u8 myAddress; //slave address
                    bool isFirstWr;     //is a register address write
                    u8* regPtr;         //current register address (pointer)
                }
blinkerS        = { {0}, 0, 0, 0x51, false, 0 };

                bool
blinkerCallbackS(twis_irqstate_t state, u8 statusReg)
                {
                //keep regPtr inside the range of registers that can write/read
                if( blinkerS.regPtr > &blinkerS.offTime ||
                    blinkerS.regPtr < &blinkerS.ram[0] ) blinkerS.regPtr = &blinkerS.ram[0];

                bool ret = true; //assume ok to continue transaction

                switch( state ) {
                    //check address here, could be general call (0) or maybe we
                    //have a second address or address mask
                    case TWIS_ADDRESSED:
                    if( twis0_lastAddress() != blinkerS.myAddress ) ret = false; //for us?
                    else blinkerS.isFirstWr = true; //yes, expect a register address write
                    break;

                    case TWIS_MREAD: //master read, so slave writes
                    twis0_write( *blinkerS.regPtr++ );
                    break;

                    case TWIS_MWRITE: { //parens so we can create a var inside case without error
                        u8 v = twis0_read();
                        if( blinkerS.isFirstWr ){ //if first write, is a register address write
                            blinkerS.isFirstWr = false;
                            blinkerS.regPtr = &blinkerS.ram[v]; //ram is base address 0, so v is offset from that
                            break;                            //regPtr will be validated in the next isr
                            }
                        //else is a register write
                        *blinkerS.regPtr++ = v;
                        }
                    break;

                    case TWIS_STOPPED:
                    case TWIS_ERROR:
                    ret = false;
                    break;

                    }
                return ret;
                }

                static void
blinkerRunS     (u8 n)
                {
                for( u8 i = 0; i < n; i++ ){
                    ledOnMS( blinkerS.onTime * 10 );
                    ledOffMS( blinkerS.offTime * 10 );
                    }
                }
/*------------------------------------------------------------------------------
    twi0 master communications to slave device
    assume we have no access to the blinker struct above (which we would not
    normally have access to), so create any needed values (enum works good)
------------------------------------------------------------------------------*/
                //enums for Blinker device
                enum { BLINKER_SLAVE_ADDRESS = 0x51,    //slave address
                       BLINKER_RAM = 0,                 //register addresses
                       BLINKER_ONTIME = 8,
                       BLINKER_OFFTIME };

                //master to slave (blinker)
                static bool
blinkerWriteM   (u8 reg, const u8* v, u8 vlen)
                {
                twim0_baud( F_CPU, 100000ul );
                twim0_on( BLINKER_SLAVE_ADDRESS );
                twim0_writeWrite( &reg, 1, v, vlen ); //write register address, write value(s)
                bool ret = twim0_waitUS( 3000 );
                twim0_off();
                return ret;
                }

                static bool
blinkerReadM    (u8 reg, u8* v, u8 vlen)
                {
                twim0_baud( F_CPU, 100000ul );
                twim0_on( BLINKER_SLAVE_ADDRESS );
                twim0_writeRead( &reg, 1, v, vlen ); //write register address, read value(s)
                bool ret = twim0_waitUS( 3000 );
                twim0_off();
                return ret;
                }

                static void
blinkerResetM   ()
                {
                //special case for bus recovery since we are using the same pins
                //for master/slave in this example, so we have to get the slave
                //to also release the pins so we can access the pins via the
                //port peripheral
                twis0_off();
                twim0_busRecovery();
                twis0_on( blinkerS.myAddress, blinkerCallbackS ); //turn the slave back on
                //turn led on for 10 sec to indicate error
                ledOnMS( 10000 );
                ledOffMS( 0 );
                }

/*------------------------------------------------------------------------------
    main
------------------------------------------------------------------------------*/
                int
main            ()
                {

                //setup blinker slave device
                twis0_on( blinkerS.myAddress, blinkerCallbackS );
                sei();

                //blinker device has ram registers, will use ram[0]
                //to store a value so we can test reading the slave also
                //if fails, keep trying
                u8 blinkN = 5; //blink N times in loop below
                while( ! blinkerWriteM(BLINKER_RAM, &blinkN, 1) ){ //write 1 byte, 5 -> ram[0]
                    blinkerResetM();
                    }

                //table of led on/off value pairs
                const u8 onOffTbl[] = {
                    2, 40,      //20ms on, 400ms off
                    100, 100    //1000ms on, 1000ms off
                    };
                const u8 tblSize = sizeof(onOffTbl)/sizeof(onOffTbl[0]);
                u8 idx = 0;

                //loop every 2 seconds
                while( waitMS(2000), 1 ) {
                    //keep idx inside table
                    if( idx >= tblSize ) idx = 0;
                    //write 2 values starting at ONTIME register (master->slave)
                    if( ! blinkerWriteM(BLINKER_ONTIME, &onOffTbl[idx], 2) ){
                        blinkerResetM();
                        continue;
                        }
                    //next pair
                    idx += 2;

                    //get value from register ram[0] (should be same as blinkN initial value above)
                    //if any error, do bus recovery and turn on led for 10 seconds
                    if( ! blinkerReadM( BLINKER_RAM, &blinkN, 1) ){
                        blinkerResetM();
                        continue; //start over
                        }

                    //blink N times (slave Blinker is doing this)
                    blinkerRunS( blinkN );

                    } //while

                } //main

