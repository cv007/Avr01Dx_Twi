#include <stdint.h>
#include <stdbool.h>
#include "fcpu.h"
#include <util/delay.h>

#include "ds3231.h"
#include "twis.h"
#include "twim.h"


/*------------------------------------------------------------------------------
    slave - address 0x44, respond to read command 0x55 with SREG value
------------------------------------------------------------------------------*/
bool twisCallback(twis_irqstate_t state, u8 status){
    bool ret = true;
    switch( state ){
        case ADDRESSED: //1
            ret = twis_lastAddress() == 0x44;
            break;
        case MREAD: //3
            twis_write( SREG );
            break;
        case MWRITE: //2
            ret = (twis_read() == 0x55);
            break;
        case STOPPED: //4
            break;

    }
    return ret;
}

void testSlave(){
    twis_defaultPins();             //slave pins
    twis_init( 0x44, twisCallback );//0x44, callback function above

    twim_address( 0x44 );           //set to our slave address
    twim_baud( F_CPU, 100000ul );   //100kHz
    twim_on();                      //on
    u8 wrbuf[1] = { 0x55 };         //command
    u8 rdbuf[1];                    //read 1 byte
    twim_writeRead( wrbuf, 1, rdbuf, 1 ); //do transaction, 1 write, 1 read
    twim_wait( 3000 );              //wait for complettion or timeout (3ms)

}


int main(){

    while(1){
        testSlave();
        _delay_ms(10);

        u8 sec;
        ds3231_seconds( &sec );
        _delay_ms(1000);
    }
}

