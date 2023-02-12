#pragma once
//======================================================================
//  MyAvr.h - global includes/defines, etc.
//======================================================================
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

//======================================================================
//  global F_CPU for delays, or any timing calculations
//======================================================================
#define F_CPU 3333333ul
#include <util/delay.h>

typedef uint8_t  u8;
typedef int8_t   i8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;