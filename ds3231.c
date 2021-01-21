//======================================================================
//  ds3231.c
//======================================================================
#include <stdint.h>
#include <stdbool.h>

#include "fcpu.h"
#include "ds3231.h"
#include "twim.h"

    //==========
    // private:
    //==========

typedef uint8_t u8;
typedef uint16_t u16;

enum { SLAVE_ADDRESS = 0x68 };
enum { NORMAL = 100000ul, FAST = 4000000ul };
enum { US_TIMEOUT = 2000 };
enum { TWIPINS = 0 }; //0=defualt pins,1=alternate pins

//normally would have a struct of all register bits, just seconds here
typedef union {
    u8 all[0x12+1];
    struct {
        //0x00
        u8 seconds1  : 4;
        u8 seconds10 : 4;
        //0x01, etc.
    };
} registers_t;

registers_t registers;

static void init(){
    if( TWIPINS ) twim_altPins(); else twim_defaultPins();
    twim_address( SLAVE_ADDRESS );
    twim_baud( F_CPU, NORMAL );
    twim_on();
}
static bool readAll(){
    uint8_t wrbuf[1] = { 0 }; //reg address start
    twim_writeRead( wrbuf, 1, registers.all, sizeof registers.all );
    return twim_wait( US_TIMEOUT );
}

    //==========
    // public:
    //==========

//blocking, with timeout
bool ds3231_seconds(u8* v){
    init();
    if( ! readAll() ) return false;
    *v = registers.seconds10 * 10 + registers.seconds1;
    return true;
}


