                //======================================================================
                //  main.c
                //
                //  demonstrate twi0 usage, both master and slave communicating on same
                //  device
                //
                //  tested with an ATtiny817 Curiosity Nano (led is PA3, inverted)
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
                    led - PA3 (inverted) in this case
                ------------------------------------------------------------------------------*/
                typedef struct { PORT_t* port; u8 pin; bool invert; } const pin_t;

                static pin_t led = { &PORTA, 3, true };  //PA3, low is on

                static inline void
pin_high        (pin_t p){ p.port->DIRSET = 1<<p.pin; p.port->OUTSET = 1<<p.pin; }
                static inline void
pin_low         (pin_t p){ p.port->DIRSET = 1<<p.pin; p.port->OUTCLR = 1<<p.pin; }
                static inline void
pin_on          (pin_t p){ p.invert ? pin_low(p) : pin_high(p); }
                static inline void
pin_off         (pin_t p){ p.invert ? pin_high(p) : pin_low(p); }



                /*------------------------------------------------------------------------------
                    Blinker I2C device - example i2c device on this mcu

                        twi0 slave address 0x51

                        slave device has 1 register at address 0x00
                        which sets or reads the state of the led
                        if register 0x00 is written to a value >0, then the led will be turned on
                        if register 0x00 is written to a value =0, then the led will be turned off
                        reading register 0x00 will return the current state of the led

                ------------------------------------------------------------------------------*/

                enum { BLINKER_SLAVE_ADDRESS = 0x51 };

                bool
blinkerCallback (twis_irqstate_t state, u8 statusReg)
                {
                static bool is1stwrite;
                static u8 led_state;
                static u8 reg_addr; //only looking for 0x00

                bool ret = true; //assume ok to continue transaction

                switch( state ) {
                    //check address here, could be general call (0) or maybe we
                    //have a second address or address mask
                    case TWIS_ADDRESSED:
                        if( twis0_lastAddress() != BLINKER_SLAVE_ADDRESS ) ret = false; //for us?
                        else is1stwrite = true; //yes, expect a register address write
                        break;

                    case TWIS_MREAD: //master read, so slave writes
                        twis0_write( led_state );
                        break;

                    case TWIS_MWRITE: { //parens so we can create a var inside case without error
                        u8 v = twis0_read();
                        if( is1stwrite ){ //if first write, is a register address write
                            is1stwrite = false;
                            reg_addr = v; //register address (should be 0x00)
                            break;
                            }
                        if( reg_addr != 0 ){
                            ret = false; //invalid register address
                            break;
                            }
                        led_state = v;
                        reg_addr = 0xFF; //invalidate any more writes in same transaction
                        if( led_state ) pin_on(led); else pin_off(led);
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
                    twi0 master communications to slave device
                ------------------------------------------------------------------------------*/

                //master to slave (blinker)
                static bool
blinkerWriteM   (u8 const reg, const u8 v)
                {
                twim0_baud( F_CPU, 100000ul );
                twim0_on( BLINKER_SLAVE_ADDRESS );
                twim0_writeWrite( &reg, 1, &v, 1 ); //write register address, write value
                bool ret = twim0_waitUS( 3000 );
                twim0_off();
                return ret;
                }

                static bool
blinkerReadM    (u8 reg, u8* v)
                {
                twim0_baud( F_CPU, 100000ul );
                twim0_on( BLINKER_SLAVE_ADDRESS );
                twim0_writeRead( &reg, 1, v, 1 ); //write register address, read value
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
                twis0_off(); //need slave off in this example
                twim0_bus_recovery(); //this will first turn off master
                twis0_on( BLINKER_SLAVE_ADDRESS, blinkerCallback ); //turn the slave back on
                }


/*------------------------------------------------------------------------------
    main
------------------------------------------------------------------------------*/
                int
main            ()
                {

                //setup blinker slave device
                twis0_on( BLINKER_SLAVE_ADDRESS, blinkerCallback );
                u8 led_state = 0;
                sei();

                //loop every 1/2 second
                while( waitMS(500), 1 ) {
                    if( ! blinkerWriteM( 0x00, led_state ) ){
                        blinkerResetM(); //if any error, do bus recovery
                        continue;
                        }

                    //get value from register (should be same as what we just wrote)
                    u8 tmp;
                    if( ! blinkerReadM(0x00, &tmp) || tmp != led_state ){
                        blinkerResetM(); //if any error, do bus recovery
                        continue; //start over
                        }

                    //we want led to change state (on->off, off->on)
                    led_state ^= 1;

                    //send out bus recovery so we can view with logic analyzer
                    waitMS(10);
                    blinkerResetM();

                    } //while

                } //main

