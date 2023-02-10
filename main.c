#include "MyAvr.h"

#include "twis0.h"
#include "twim0.h"

static u8 led_onMSx10;
static u8 led_offMSx10;

enum { TWIS_ON_TIMEx10ms = 8, TWIS_OFF_TIMEx10ms, TWIS0_MY_ADDRESS = 0x51 };

typedef struct { PORT_t* port; u8 pin; bool invert; } pin_t;
static const pin_t led = { &PORTB, 5, true }; //PB5, low is on


/*------------------------------------------------------------------------------
    twi0 slave - simulate some i2c device, led blinker device in this case

        slave address 0x51

        our slave device has a register at address 0x08 named ON_TIMEx10ms,
            and an address of 0x09 named OFF_TIMEx10ms
        the ON_TIMEx10ms value will determine the led blink on time-
            0 = off
            1 = 10ms on
            255 = 2550ms on
        the OFF_TIMEx10ms value will determine the led off time (if ON_TIMEx10ms not 0)-
            0 = off
            1 = 10ms off
            255 = 2550ms off

        register address will auto increment

        to write a new value to ON_TIMEx10ms or OFF_TIMEx10ms
            write 0x08 or 0x09, write new value
        to read the current value of ON_TIMEx10ms
            write 0x08 or 0x09, read value
        to write a new value to ON_TIMEx10ms and OFF_TIMEx10ms
            write 0x08, write new ON_TIMEx10ms value, write new OFF_TIMEx10ms
        to read the current value of ON_TIMEx10ms and OFF_TIMEx10ms
            write 0x08, read ON_TIMEx10ms value, read OFF_TIMEx10ms

------------------------------------------------------------------------------*/
                bool
twis0Callback   (twis_irqstate_t state, u8 statusReg){

                static bool isFirstWr_; //first write after start? is register address
                static u8 registerN_;

                bool ret = true; //assume ok to continue transaction
                switch( state ) {
                    //check address here, could be general call (0) or maybe we
                    //have a second address or address mask
                    case TWIS_ADDRESSED:
                        if( twis0_lastAddress() != TWIS0_MY_ADDRESS ) ret = false; //for us?
                        else isFirstWr_ = true;
                        break;
                    case TWIS_MREAD:
                        {
                        u8 v = 0; //assume invalid register, which will send 0
                        if( registerN_ == TWIS_ON_TIMEx10ms ) v = led_onMSx10;
                        else if( registerN_ == TWIS_OFF_TIMEx10ms ) v = led_offMSx10;
                        twis0_write( v );
                        registerN_++;
                        }
                        break;
                    case TWIS_MWRITE:
                        {
                        u8 v = twis0_read();
                        //if first write, is a register address write
                        if( isFirstWr_ ){
                            isFirstWr_ = false;
                            registerN_ = v;
                            break;
                            }
                        //else is a register write
                        if( registerN_ == TWIS_ON_TIMEx10ms ) led_onMSx10 = v;
                        else if( registerN_ == TWIS_OFF_TIMEx10ms ) led_offMSx10 = v;
                        registerN_++;
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
pinSet          (const pin_t p, bool on)
                {
                u8 pinbm = 1<<(p.pin & 7);
                p.port->DIRSET = pinbm;
                if( p.invert ) (&p.port->PIN0CTRL)[p.pin] |= 0x80;
                on ? (p.port->OUTSET = pinbm) : (p.port->OUTCLR = pinbm);
                }

                static void
waitMS          (u16 ms){ while( ms-- ) _delay_ms(1); }

                static bool
blinkerWrite    (u8 reg, const u8* v, u8 vlen)
                {
                twim0_stdPins();
                twim0_baud( F_CPU, 100000ul );
                twim0_on( TWIS0_MY_ADDRESS );
                twim0_writeWrite( &reg, 1, v, vlen ); //write register address, write value(s)
                bool ret = twim0_waitUS( 3000 );
                twim0_off();
                return ret;
                }

//                static bool
//blinkerRead     (u8 reg, u8* v)
//                {
//                twim0_stdPins();
//                twim0_baud( F_CPU, 100000ul );
//                twim0_on( twis0_my_address );
//                twim0_writeRead( &reg, 1, v, 1 ); //write register address, read value
//                bool ret = twim0_waitUS( 3000 );
//                twim0_off();
//                return ret;
//                }

                int
main            ()
                {

                twis0_stdPins();
                twis0_init( TWIS0_MY_ADDRESS, twis0Callback );
                sei();

                static const u8 onOffTbl[] = {
                    2, 20,  //20ms on, 200ms off
                    100, 100 //1000ms on, 1000ms off
                    };
                u8 i = 0;

                while(1) {

                    if( i >= sizeof(onOffTbl)/sizeof(onOffTbl[0]) ) i = 0;

                    //write 2 values starting at ON time register
                    if( ! blinkerWrite(TWIS_ON_TIMEx10ms, &onOffTbl[i], 2) ){
                        _delay_ms( 1000 ); //failed, delay
                        continue; //and try again
                        }
                    i += 2;

                    //blink 5 times
                    for( u8 n = 0; n < 5; n++ ){
                        pinSet( led, 1 );
                        waitMS( led_onMSx10 * 10 );
                        pinSet( led, 0 );
                        waitMS( led_offMSx10 * 10 );
                        }

                    //delay between blinks
                    waitMS( 2000 );

                    }

            }

