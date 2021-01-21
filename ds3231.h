#pragma once
//======================================================================
//  ds3231.h
//======================================================================
#include <stdint.h>

typedef uint8_t u8;

bool ds3231_seconds(u8* v); //return true = success, v = value
