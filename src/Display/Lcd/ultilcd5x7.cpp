/*
 * ultilcd5x7.cpp
 *
 *  Created on: 2 Mar 2021
 *      Author: Ethan, copied from https://github.com/Ultimaker/Ultimaker2Marlin/blob/master/Marlin/UltiLCD2_low_lib.cpp
 */

#include "Lcd.h"

#if SUPPORT_12864_LCD

const uint8_t font[] =
{
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00,// (space)
	0x05, 0x00, 0x00, 0x5F, 0x00, 0x00,// !
	0x05, 0x00, 0x07, 0x00, 0x07, 0x00,// "
	0x05, 0x14, 0x7F, 0x14, 0x7F, 0x14,// #
	0x05, 0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
	0x05, 0x23, 0x13, 0x08, 0x64, 0x62,// %
	0x05, 0x36, 0x49, 0x55, 0x22, 0x50,// &
	0x05, 0x00, 0x05, 0x03, 0x00, 0x00,// '
	0x05, 0x00, 0x1C, 0x22, 0x41, 0x00,// (
	0x05, 0x00, 0x41, 0x22, 0x1C, 0x00,// )
	0x05, 0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
	0x05, 0x08, 0x08, 0x3E, 0x08, 0x08,// +
	0x05, 0x00, 0x50, 0x30, 0x00, 0x00,// ,
	0x05, 0x08, 0x08, 0x08, 0x08, 0x08,// -
	0x05, 0x00, 0x60, 0x60, 0x00, 0x00,// .
	0x05, 0x20, 0x10, 0x08, 0x04, 0x02,// /
	0x05, 0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
	0x05, 0x00, 0x42, 0x7F, 0x40, 0x00,// 1
	0x05, 0x42, 0x61, 0x51, 0x49, 0x46,// 2
	0x05, 0x21, 0x41, 0x45, 0x4B, 0x31,// 3
	0x05, 0x18, 0x14, 0x12, 0x7F, 0x10,// 4
	0x05, 0x27, 0x45, 0x45, 0x45, 0x39,// 5
	0x05, 0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
	0x05, 0x01, 0x71, 0x09, 0x05, 0x03,// 7
	0x05, 0x36, 0x49, 0x49, 0x49, 0x36,// 8
	0x05, 0x06, 0x49, 0x49, 0x29, 0x1E,// 9
	0x05, 0x00, 0x36, 0x36, 0x00, 0x00,// :
	0x05, 0x00, 0x56, 0x36, 0x00, 0x00,// ;
	0x05, 0x00, 0x08, 0x14, 0x22, 0x41,// <
	0x05, 0x14, 0x14, 0x14, 0x14, 0x14,// =
	0x05, 0x41, 0x22, 0x14, 0x08, 0x00,// >
	0x05, 0x02, 0x01, 0x51, 0x09, 0x06,// ?
	0x05, 0x32, 0x49, 0x79, 0x41, 0x3E,// @
	0x05, 0x7E, 0x11, 0x11, 0x11, 0x7E,// A
	0x05, 0x7F, 0x49, 0x49, 0x49, 0x36,// B
	0x05, 0x3E, 0x41, 0x41, 0x41, 0x22,// C
	0x05, 0x7F, 0x41, 0x41, 0x22, 0x1C,// D
	0x05, 0x7F, 0x49, 0x49, 0x49, 0x41,// E
	0x05, 0x7F, 0x09, 0x09, 0x01, 0x01,// F
	0x05, 0x3E, 0x41, 0x41, 0x51, 0x32,// G
	0x05, 0x7F, 0x08, 0x08, 0x08, 0x7F,// H
	0x05, 0x00, 0x41, 0x7F, 0x41, 0x00,// I
	0x05, 0x20, 0x40, 0x41, 0x3F, 0x01,// J
	0x05, 0x7F, 0x08, 0x14, 0x22, 0x41,// K
	0x05, 0x7F, 0x40, 0x40, 0x40, 0x40,// L
	0x05, 0x7F, 0x02, 0x04, 0x02, 0x7F,// M
	0x05, 0x7F, 0x04, 0x08, 0x10, 0x7F,// N
	0x05, 0x3E, 0x41, 0x41, 0x41, 0x3E,// O
	0x05, 0x7F, 0x09, 0x09, 0x09, 0x06,// P
	0x05, 0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
	0x05, 0x7F, 0x09, 0x19, 0x29, 0x46,// R
	0x05, 0x46, 0x49, 0x49, 0x49, 0x31,// S
	0x05, 0x01, 0x01, 0x7F, 0x01, 0x01,// T
	0x05, 0x3F, 0x40, 0x40, 0x40, 0x3F,// U
	0x05, 0x1F, 0x20, 0x40, 0x20, 0x1F,// V
	0x05, 0x7F, 0x20, 0x18, 0x20, 0x7F,// W
	0x05, 0x63, 0x14, 0x08, 0x14, 0x63,// X
	0x05, 0x03, 0x04, 0x78, 0x04, 0x03,// Y
	0x05, 0x61, 0x51, 0x49, 0x45, 0x43,// Z
	0x05, 0x00, 0x00, 0x7F, 0x41, 0x41,// [
	0x05, 0x02, 0x04, 0x08, 0x10, 0x20,// "\"
	0x05, 0x41, 0x41, 0x7F, 0x00, 0x00,// ]
	0x05, 0x04, 0x02, 0x01, 0x02, 0x04,// ^
	0x05, 0x40, 0x40, 0x40, 0x40, 0x40,// _
	0x05, 0x00, 0x01, 0x02, 0x04, 0x00,// `
	0x05, 0x20, 0x54, 0x54, 0x54, 0x78,// a
	0x05, 0x7F, 0x48, 0x44, 0x44, 0x38,// b
	0x05, 0x38, 0x44, 0x44, 0x44, 0x20,// c
	0x05, 0x38, 0x44, 0x44, 0x48, 0x7F,// d
	0x05, 0x38, 0x54, 0x54, 0x54, 0x18,// e
	0x05, 0x08, 0x7E, 0x09, 0x01, 0x02,// f
	0x05, 0x08, 0x14, 0x54, 0x54, 0x3C,// g
	0x05, 0x7F, 0x08, 0x04, 0x04, 0x78,// h
	0x05, 0x00, 0x44, 0x7D, 0x40, 0x00,// i
	0x05, 0x20, 0x40, 0x44, 0x3D, 0x00,// j
	0x05, 0x00, 0x7F, 0x10, 0x28, 0x44,// k
	0x05, 0x00, 0x41, 0x7F, 0x40, 0x00,// l
	0x05, 0x7C, 0x04, 0x18, 0x04, 0x78,// m
	0x05, 0x7C, 0x08, 0x04, 0x04, 0x78,// n
	0x05, 0x38, 0x44, 0x44, 0x44, 0x38,// o
	0x05, 0x7C, 0x14, 0x14, 0x14, 0x08,// p
	0x05, 0x08, 0x14, 0x14, 0x18, 0x7C,// q
	0x05, 0x7C, 0x08, 0x04, 0x04, 0x08,// r
	0x05, 0x48, 0x54, 0x54, 0x54, 0x20,// s
	0x05, 0x04, 0x3F, 0x44, 0x40, 0x20,// t
	0x05, 0x3C, 0x40, 0x40, 0x20, 0x7C,// u
	0x05, 0x1C, 0x20, 0x40, 0x20, 0x1C,// v
	0x05, 0x3C, 0x40, 0x30, 0x40, 0x3C,// w
	0x05, 0x44, 0x28, 0x10, 0x28, 0x44,// x
	0x05, 0x0C, 0x50, 0x50, 0x50, 0x3C,// y
	0x05, 0x44, 0x64, 0x54, 0x4C, 0x44,// z
	0x05, 0x00, 0x08, 0x36, 0x41, 0x00,// {
	0x05, 0x00, 0x00, 0x7F, 0x00, 0x00,// |
	0x05, 0x00, 0x41, 0x36, 0x08, 0x00,// }
	0x05, 0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
	0x05, 0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
};

extern const LcdFont font5x7 =
{
	font,	                // font data
	0x0020,					// first character code
	0x007F,					// last character code
	7,						// row height in pixels
	5,						// character width in pixels
	1						// number of space columns between characters before kerning
};

#endif

// End