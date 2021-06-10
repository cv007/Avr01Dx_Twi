#include "MyAvr.h"

#include "twis.h"
#include "twim.h"

#if 0
#include "ds3231.h"
/*------------------------------------------------------------------------------
    slave - address 0x44, respond to read command 0x55 with v value
------------------------------------------------------------------------------*/
bool twisCallback(twis_irqstate_t state, u8 statusReg){
    bool ret = true;
    static u8 v;

    switch( state ) {
        case TWIS_ADDRESSED: //1
            ret = twis_lastAddress() == 0x44; //for us?
            break;
        case TWIS_MREAD: //3
            twis_write( v++ ); //respond
            break;
        case TWIS_MWRITE: //2
            ret = (twis_read() == 0x55); //valid command?
            break;
        case TWIS_STOPPED:
        case TWIS_ERROR:
            ret = false;
            break;
        }
    return ret;
}

//send command to slave
void testSlave(){
    twis_defaultPins();             //slave pins
    twis_init( 0x44, twisCallback );//0x44, callback function above

    twim_defaultPins();             //master pins (same as slave)
    twim_baud( F_CPU, 100000ul );   //100kHz
    twim_on( 0x44 );                //on, slave address 0x44
    u8 wrbuf[1] = { 0x55 };         //command
    u8 rdbuf[1];                    //read 1 byte
    twim_writeRead( wrbuf, 1, rdbuf, 1 ); //do transaction, 1 write, 1 read
    twim_waitUS( 3000 );            //wait for complettion or timeout (3ms)
}


//watch w/logic analyzer
int main(){

    while(1){

        testSlave();
        _delay_ms(10);

        u8 sec;
        ds3231_seconds( &sec );
        _delay_ms(1000);

        }

}
#endif

#if 1
/*------------------------------------------------------------------------------
    slave - address 0x51
------------------------------------------------------------------------------*/
bool twisCallback(twis_irqstate_t state, u8 statusReg){
    bool ret = true;

    switch( state ) {
        case TWIS_ADDRESSED:
            ret = (twis_lastAddress() == 0x51); //for us?
            break;
        case TWIS_MREAD:
            twis_write( statusReg ); //respond (just send status register value)
            break;
        case TWIS_MWRITE:
        case TWIS_STOPPED:
        case TWIS_ERROR:
            ret = false;
            break;

        }
    return ret;
}

//send command to slave
void testSlave(){
    twis_defaultPins();             //slave pins
    twis_init( 0x51, twisCallback );//0x51, callback function above

    twim_defaultPins();             //master pins (same as slave)
    twim_baud( F_CPU, 100000ul );   //100kHz
    twim_on( 0x51 );                //on, slave address 0x51
    u8 rdbuf[5];                    //read 5 bytes
    twim_read( rdbuf, sizeof(rdbuf) );//do transaction, read n bytes
    twim_waitUS( 3000 );            //wait for complettion or timeout (3ms)
}


//watch w/logic analyzer
int main(){

    sei();
    while(1) {

        testSlave();
        _delay_ms(100);

        }

}
#endif