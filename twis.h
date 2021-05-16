#pragma once
/*------------------------------------------------------------------------------
    twis.h - Twi slave
------------------------------------------------------------------------------*/
#include "MyAvr.h"

typedef enum { ADDRESSED, MREAD, MWRITE, STOPPED } twis_irqstate_t;
typedef bool(*twis_callback_t)(twis_irqstate_t state, u8 statusReg);

void twis_off           ();
void twis_write         (u8 value);
u8   twis_read          ();
u8   twis_lastAddress   ();
void twis_address2      (u8 SlaveAddress2);
void twis_init          (u8 SlaveAddress, twis_callback_t callbackFunction);
void twis_defaultPins   ();
void twis_altPins       ();


