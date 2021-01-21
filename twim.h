#pragma once
//======================================================================
//  twim.h - Twi0, master - avr mega0, tiny0/1, da
//======================================================================
#include <avr/io.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;

typedef void (*twim_callbackT)(void);

void twim_address       (u8 address);
void twim_off           ();
void twim_on            ();
bool twim_isBusy        ();
bool twim_lastResultOK  ();
void twim_callback      (twim_callbackT myFunction);
void twim_writeRead     (const u8* write_buffer, u16 write_length,
                         u8* read_buffer, u16 read_length);
void twim_writeWrite    (const u8* write_buffer, u16 write_length,
                         const u8* write_buffer2, u16 write_length2);
void twim_write         (const u8* write_buffer, u16 write_length);
void twim_read          (u8* read_buffer, u16 read_length);
bool twim_wait          (u16 us);
void twim_defaultPins   ();
void twim_altPins       ();

//baud
inline __attribute((always_inline)) static
void twim_baud   (uint32_t cpuHz, uint32_t twiHz){
                            int32_t v = cpuHz/twiHz/2.0 + 0.99 - 5;
                            TWI0.MBAUD = v >= 0 ? v : 0;
                        }
