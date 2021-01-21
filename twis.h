#pragma once
/*------------------------------------------------------------------------------
    twis.h - Twi slave
------------------------------------------------------------------------------*/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;

typedef enum { ADDRESSED, MREAD, MWRITE, STOPPED } twis_irqstate_t;
typedef bool(*twis_callback_t)(twis_irqstate_t state, u8 status);

void twis_off           ();
void twis_write         (u8 v);
u8   twis_read          ();
u8   twis_lastAddress   ();
void twis_address2      (u8 addr2);
void twis_init          (u8 addr, twis_callback_t cb);
void twis_defaultPins   ();
void twis_altPins       ();
