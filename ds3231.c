//======================================================================
//  ds3231.c
//======================================================================
#include "MyAvr.h"

#include "ds3231.h"
#include "twim.h"

    //==========
    // private:
    //==========

    enum { SLAVE_ADDRESS = 0x68 };
    enum { NORMAL = 100000ul, FAST = 4000000ul };
    enum { US_TIMEOUT = 2000 };
    enum { TWIPINS_ALT = 0 }; //0=default pins,1=alternate pins

    //normally would have a struct of all register bits,
    //but just have seconds here for a simple example
    typedef union {
    u8 all[0x12+1]; //all registers, 0x00-0x12
    struct {
        //0x00
        u8 seconds1  : 4;
        u8 seconds10 : 4;
        //0x01, etc.
    };
    } registers_t;

    registers_t registers;


static void init        ()
                        {
                        if( TWIPINS_ALT ) twim_altPins(); else twim_defaultPins();
                        twim_baud( F_CPU, NORMAL );
                        twim_on( SLAVE_ADDRESS );
                        }

static bool readAll     ()
                        {
                        uint8_t wrbuf[1] = { 0 }; //reg address start
                        twim_writeRead( wrbuf, 1, registers.all, sizeof registers.all );
                        return twim_waitUS( US_TIMEOUT );
                        }

    //==========
    // public:
    //==========

                        //blocking, with timeout
bool    ds3231_seconds  (u8* seconds)
                        {
                        init();
                        if( ! readAll() ) return false;
                        *seconds = registers.seconds10 * 10 + registers.seconds1;
                        return true;
                        }


