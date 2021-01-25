#pragma once
//======================================================================
//  ds3231.h
//======================================================================
#include <stdint.h>

typedef uint8_t u8;

bool ds3231_seconds(u8* seconds); //return true = success
