/*
 * lcd1309.cpp
 *
 *  Created on: Mar 8, 2020
 *      Author: Ethan Tillison
 *
 *  Derived from lcd7920.h
 */

#include "lcd1309.h"

#if SUPPORT_12864_LCD

#include "Wire.h"
#include "Pins.h"
#include "Tasks.h"
#include "Hardware/IoPorts.h"

#include "RepRap.h"

constexpr uint8_t I2CLcdSendCommand = 0x00;
constexpr uint8_t I2CLcdSendData    = 0x40;

constexpr uint8_t LcdCommandContrast             = 0x81;
constexpr uint8_t LcdCommandFullDisplayOnDisable = 0xA4;
constexpr uint8_t LcdCommandFullDisplayOnEnable  = 0xA5;
constexpr uint8_t LcdCommandInvertDisable        = 0xA6;
constexpr uint8_t LcdCommandInvertEnable         = 0xA7;
constexpr uint8_t LcdCommandDisplayOff           = 0xAE;
constexpr uint8_t LcdCommandDisplayOn            = 0xAF;
constexpr uint8_t LcdCommandNop                  = 0xE3;
constexpr uint8_t LcdCommandLockCommands         = 0xFD;
constexpr uint8_t LcdCommandSetAddressingMode    = 0x20;

constexpr unsigned int LcdCommandDelayMicros = 10;
constexpr unsigned int LcdDataDelayMicros = 10;			// delay between sending data bytes
constexpr unsigned int LcdDisplayClearDelayMillis = 3;	// 1.6ms should be enough

/**
 * Send data over I2C. Must hold lock first.
 */
static void Transfer(uint16_t address, uint8_t *buffer, size_t numToWrite, size_t numToRead) {
	delayMicroseconds(LcdCommandDelayMicros);
	I2C_IFACE.Transfer(address, buffer, numToWrite, numToRead, TwoWire::DefaultWaitForStatusFunc);
}


Lcd1309::Lcd1309(const LcdFont * const fnts[], size_t nFonts)
	: fonts(fnts), numFonts(nFonts), currentFontNumber(0), numContinuationBytesLeft(0), textInverted(false) {
	_ssd1309_addr = DEFAULT_ADDRESS;
}

void Lcd1309::setAddress(uint8_t addr) {
	_ssd1309_addr = addr;
}

void Lcd1309::Init()
{
	numContinuationBytesLeft = 0;
	startRow = NumRows;
	startCol = NumCols;
	endRow = endCol = nextFlushCol = 0;

	IoPort::SetPinMode(LcdResetPin, OUTPUT_HIGH);

	// Starting sequence from Ultimaker2Marlin
	// https://github.com/Ultimaker/Ultimaker2Marlin/blob/master/Marlin/UltiLCD2_low_lib.cpp
	IoPort::WriteDigital(LcdResetPin, 0); // Drive the reset pin low
	delay(1);
	IoPort::WriteDigital(LcdResetPin, 1); // then high to reset the display
	delay(1);

	I2C::Init();

	{
		uint8_t i2cBytes[24] = {
				I2CLcdSendCommand,
				LcdCommandLockCommands, 0x12, // Unlock Commands
				LcdCommandDisplayOff,
				0xD5, 0xA0, // Clock divider/freq.
				0xA8, 0x3F, // Multiplex ratio
				0xD3, 0x00, // Display offset
				0x40, // Start line
				0xA1, // Segment remap
				0xC8, 0xDA, 0x12, // COM Scan direction
				LcdCommandContrast, 0xDF,
				0xD9, 0x82, // Precharge period
				0xDB, 0x34, // VCOMH Deselect level
				LcdCommandSetAddressingMode, 0x01, // Vertical addressing (each successive data byte will appear in the column below the previous byte)
				LcdCommandFullDisplayOnDisable // Turn the 'all pixels on' mode off
		};

		MutexLocker lock(Tasks::GetI2CMutex());
		Transfer(_ssd1309_addr, i2cBytes, 24, 0);
	}

	Clear();
	FlushAll();

	{
		uint8_t i2cBytes[2] = {I2CLcdSendCommand, LcdCommandDisplayOn};

		MutexLocker lock(Tasks::GetI2CMutex());
		Transfer(_ssd1309_addr, i2cBytes, 2, 0);
	}

	currentFontNumber = 0;
}

void Lcd1309::SetFont(size_t newFont)
{
	if (newFont < numFonts)
	{
		currentFontNumber = newFont;
	}
}

// Get the current font height
PixelNumber Lcd1309::GetFontHeight() const
{
	return fonts[currentFontNumber]->height;
}

// Get the height of a specified font
PixelNumber Lcd1309::GetFontHeight(size_t fontNumber) const
{
	if (fontNumber >= numFonts)
	{
		fontNumber = currentFontNumber;
	}
	return fonts[fontNumber]->height;
}

// Flag a pixel as dirty. The r and c parameters must be no greater than NumRows-1 and NumCols-1 respectively.
// Only one pixel in each 16-bit word needs to be flagged dirty for the whole word to get refreshed.
void Lcd1309::SetDirty(PixelNumber r, PixelNumber c)
{
	if (reprap.Debug(moduleDisplay)) {
		debugPrintf("r,c=%u,%u\n", r, c);
//		if (r >= NumRows) { debugPrintf("r=%u\n", r); return; }
//		if (c >= NumCols) { debugPrintf("c=%u\n", c); return; }
	}

	if (c < startCol) { startCol = c; }
	if (c >= endCol) { endCol = c + 1; }
	if (r < startRow) { startRow = r; }
	if (r >= endRow) { endRow = r + 1; }
}

// Write a UTF8 byte.
// If textYpos is off the end of the display, then don't write anything, just update textXpos and lastCharColData
size_t Lcd1309::write(uint8_t c)
{
	if (numContinuationBytesLeft == 0)
	{
		if (c < 0x80)
		{
			return writeNative(c);
		}
		else if ((c & 0xE0) == 0xC0)
		{
			charVal = (uint32_t)(c & 0x1F);
			numContinuationBytesLeft = 1;
			return 1;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			charVal = (uint32_t)(c & 0x0F);
			numContinuationBytesLeft = 2;
			return 1;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			charVal = (uint32_t)(c & 0x07);
			numContinuationBytesLeft = 3;
			return 1;
		}
		else if ((c & 0xFC) == 0xF8)
		{
			charVal = (uint32_t)(c & 0x03);
			numContinuationBytesLeft = 4;
			return 1;
		}
		else if ((c & 0xFE) == 0xFC)
		{
			charVal = (uint32_t)(c & 0x01);
			numContinuationBytesLeft = 5;
			return 1;
		}
		else
		{
			return writeNative(0x7F);
		}
	}
	else if ((c & 0xC0) == 0x80)
	{
		charVal = (charVal << 6) | (c & 0x3F);
		--numContinuationBytesLeft;
		if (numContinuationBytesLeft == 0)
		{
			return writeNative((charVal < 0x10000) ? (uint16_t)charVal : 0x007F);
		}
		else
		{
			return 1;
		}
	}
	else
	{
		// Bad UTF8 state
		numContinuationBytesLeft = 0;
		return writeNative(0x7F);
	}
}

size_t Lcd1309::writeNative(uint16_t ch)
{
	const LcdFont * const currentFont = fonts[currentFontNumber];
	if (ch == '\n')
	{
		SetCursor(row + currentFont->height + 1, leftMargin);
	}
	else
	{
		const uint8_t startChar = currentFont->startCharacter;
		const uint8_t endChar = currentFont->endCharacter;

		if (ch < startChar || ch > endChar)
		{
			ch = 0x007F;			// replace unsupported characters by square box
		}

		uint8_t ySize = currentFont->height + padding * 2;
		const uint8_t bytesPerColumn = (ySize + 7)/8; // bytes per column in internal representation of font
		if (row >= NumRows)
		{
			ySize = 0;				// we still execute the code, so that the caller can tell how many columns the text will occupy by writing it off-screen
		}
		else if (row + ySize > NumRows)
		{
			ySize = NumRows - row;
		}

		// Calculate the location of the character in the font's array
		const uint8_t bytesPerChar = (bytesPerColumn * currentFont->width) + 1;
		const uint8_t *fontPtr = currentFont->ptr + (bytesPerChar * (ch - startChar));

		// Mask for each column of the font
		const uint16_t cmask = (1u << currentFont->height) - 1;

		// How many bits of the 8-bit row are skipped vertically
		const uint8_t ySkip = row & 7;
		// How many 8-bit rows will be written to
		const uint8_t nRows = (ySize + 7 + ySkip) / 8;

		// The number of columns for a specific character is the first byte of the array at that letter
		uint8_t nCols = *fontPtr++;
		if (lastCharColData != 0)		// if we have written anything other than spaces
		{
			uint8_t numSpaces = currentFont->numSpaces;

			// Decide whether to add a space column first (auto-kerning)
			// We don't add a space column before a space character.
			// We add a space column after a space character if we would have added one between the preceding and following characters.
			uint16_t thisCharColData = *reinterpret_cast<const uint16_t*>(fontPtr) & cmask;
			if (thisCharColData == 0)  // for characters with deliberate space column at the start, e.g. decimal point
			{
				thisCharColData = *reinterpret_cast<const uint16_t*>(fontPtr + bytesPerColumn) & cmask;
			}

			const bool kern = (numSpaces >= 2)
							? ((thisCharColData & lastCharColData) == 0)
							: (((thisCharColData | (thisCharColData << 1)) & (lastCharColData | (lastCharColData << 1))) == 0);
			if (kern)
			{
				--numSpaces;	// kern the character pair
			}
			if (numSpaces != 0 && column < rightMargin)
			{
				// Add a single space column after the character
				if (ySize != 0)
				{
					uint8_t *p = image + ((row / 8) * NumCols + column);
					for (uint8_t r = 0; r < nRows && p < (image + sizeof(image)); r++) {
					    // The top pixel of the row being written to
						uint8_t baseRow = (r + (row / 8)) * 8;
						uint8_t mask = 0xFF; // (uint8_t) (baseRow < row ? cmask << (row - baseRow) : cmask >> (baseRow - row));
						if (r == 0)
							mask &= 0xFF << (row - baseRow);
						if (r == nRows - 1)
							mask &= 0xFF >> (baseRow + 8 - row - ySize);

						const uint8_t oldVal = *p;
						const uint8_t newVal = textInverted ? oldVal | mask : oldVal & ~mask;
						if (newVal != oldVal) {
							*p = newVal;
							SetDirty(baseRow, column);
						}
						p += NumCols;
					}
				}
				++column;
			}
		}

		while (nCols != 0 && column < rightMargin)
		{
			uint32_t colData = (*reinterpret_cast<const uint16_t*>(fontPtr) << padding) & cmask;
			fontPtr += bytesPerColumn;
			if (colData != 0)
			{
				lastCharColData = colData;
			}
			if (ySize != 0)
			{
				uint8_t *p = image + ((row / 8) * NumCols + column);
				for (uint8_t r = 0; r < nRows && p < (image + sizeof(image)); r++) {
					// The top pixel of the row being written to
					uint8_t baseRow = (r + (row / 8)) * 8;
					uint8_t mask = 0xFF;
					if (r == 0)
						mask &= 0xFF << (row - baseRow);
					if (r == nRows - 1)
						mask &= 0xFF >> (baseRow + 8 - row - ySize);

					uint8_t data = (uint8_t) (baseRow < row + padding ? colData << (row + padding - baseRow) : colData >> (baseRow - row - padding));
					if (textInverted)
						data ^= mask;

					const uint8_t oldVal = *p;
					const uint8_t newVal = (oldVal & ~mask) | data;
					if (newVal != oldVal) {
						*p = newVal;
						SetDirty(baseRow, column);
					}
					p += NumCols;
				}
			}
			--nCols;
			++column;
		}

		justSetCursor = false;
	}
	return 1;
}

void Lcd1309::WriteSpaces(PixelNumber numPixels)
{
	const LcdFont * const currentFont = fonts[currentFontNumber];
	uint8_t ySize = currentFont->height + padding * 2;
	if (row >= NumRows)
	{
		ySize = 0;				// we still execute the code, so that the caller can tell how many columns the text will occupy by writing it off-screen
	}
	else if (row + ySize > NumRows)
	{
		ySize = NumRows - row;
	}

	// How many bits of the 8-bit row are skipped vertically
	const uint8_t ySkip = row & 7;
	// How many 8-bit rows will be written to
	const uint8_t nRows = (ySize + 7 + ySkip) / 8;

	while (numPixels != 0 && column < NumCols)
	{
		if (ySize != 0)
		{
			uint8_t *p = image + ((row / 8) * NumCols + column);
			for (uint8_t r = 0; r < nRows && p < (image + sizeof(image)); r++) {
				// The top pixel of the row being written to
				uint8_t baseRow = (r + (row / 8)) * 8;
				uint8_t mask = 0xFF;
				if (r == 0)
					mask &= 0xFF << (row - baseRow);
				if (r == nRows - 1)
					mask &= 0xFF >> (baseRow + 8 - row - ySize);

				const uint8_t oldVal = *p;
				const uint8_t newVal = textInverted ? oldVal | mask : oldVal & ~mask;
				if (newVal != oldVal) {
					*p = newVal;
					SetDirty(baseRow, column);
				}
				p += NumCols;
			}
		}
		--numPixels;
		++column;
	}

	lastCharColData = 0;
	justSetCursor = false;
}

// Set the left margin. This is where the cursor goes to when we print newline.
void Lcd1309::SetLeftMargin(PixelNumber c)
{
	leftMargin = min<uint8_t>(c, NumCols);
}

// Set the right margin. In graphics mode, anything written will be truncated at the right margin. Defaults to the right hand edge of the display.
void Lcd1309::SetRightMargin(PixelNumber r)
{
	rightMargin = min<uint8_t>(r, NumCols);
}

// Clear a rectangle from the current position to the right margin. The height of the rectangle is the height of the current font.
void Lcd1309::ClearToMargin()
{
	WriteSpaces(rightMargin - column);
}

// Select normal or inverted text
void Lcd1309::TextInvert(bool b)
{
	if (b != textInverted)
	{
		textInverted = b;
		if (!justSetCursor)
		{
			lastCharColData = 0xFFFF;				// force a space when switching between normal and inverted text
		}
	}
}

// Clear a rectangular block of pixels starting at rows, scol ending just before erow, ecol
void Lcd1309::Clear(PixelNumber sRow, PixelNumber sCol, PixelNumber eRow, PixelNumber eCol)
{
	if (eCol > NumCols) { eCol = NumCols; }
	if (eRow > NumRows) { eRow = NumRows; }
	if (sCol < eCol && sRow < eRow)
	{
		uint8_t sMask = ~(0xFF >> (sRow & 7));		// mask of bits we want to keep in the first byte of each column that we modify
		const uint8_t eMask = 0xFF >> (eRow & 7);	// mask of bits we want to keep in the last byte of each column that we modify
		if ((sRow & ~7) == (eRow & ~7))
		{
			sMask |= eMask;							// special case of just clearing some middle bits
		}
		for (PixelNumber col = sCol; col < eCol; col++)
		{
			uint8_t * p = image + ((sRow / 8) * NumCols + col);
			uint8_t * const endp = image + ((eRow / 8) * NumCols + col);
			*p &= sMask;
			if (p != endp)
			{
				while ((p += NumCols) < endp)
				{
					*p = 0;
				}
				if ((eCol & 7) != 0)
				{
					*p &= eMask;
				}
			}
		}

		// Flag cleared part as dirty
		if (sCol < startCol) { startCol = sCol; }
		if (eCol >= endCol) { endCol = eCol; }
		if (sRow < startRow) { startRow = sRow; }
		if (eRow >= endRow) { endRow = eRow; }

		SetCursor(sRow, sCol);
		textInverted = false;
		leftMargin = sCol;
		rightMargin = eCol;
	}
}

// Draw a bitmap. Unlike the ST7920 implementation, x0 and numCols need not be divisible by 8.
void Lcd1309::Bitmap(PixelNumber x0, PixelNumber y0, PixelNumber width, PixelNumber height, const uint8_t data[])
{
	// Due to the orientation of the bits, the easiest (and probably best) way to do this is to just draw each row
	const uint8_t *row = data;
	for (PixelNumber y = y0; y < y0 + height; y++) {
		BitmapRow(y, x0, width, row, false);
		row += (width + 7) / 8;
	}

	// Assume the whole area has changed
	if (x0 < startCol) startCol = x0;
	if (x0 + width > endCol) endCol = x0 + width;
	if (y0 < startRow) startRow = y0;
	if (y0 + height > endRow) endRow = y0 + height;
}

// Draw a single bitmap row. 'left' and 'width' do not need to be divisible by 8.
void Lcd1309::BitmapRow(PixelNumber top, PixelNumber left, PixelNumber width, const uint8_t data[], bool invert)
{
	const uint8_t mask = 1 << (top % 8);
	uint8_t *p = image + ((top / 8) * NumCols) + left;
	for (PixelNumber c = 0; c < width; c++) {
		const bool setPixelVal = (data[c / 8] & (1 << (c % 8))) != 0;
		const uint8_t oldVal = *p;
		const uint8_t newVal = setPixelVal ? oldVal | mask : oldVal & ~mask;
		if (newVal != oldVal) {
			*p = newVal;
			SetDirty(top, left + c);
		}
		p++;
	}
}

// Flush all of the dirty part of the image to the lcd. Only called during startup and shutdown.
void Lcd1309::FlushAll()
{
	while (FlushSome()) ;
}

// Flush some of the dirty part of the image to the LCD, returning true if there is more to do
bool Lcd1309::FlushSome()
{
	// See if there is anything to flush
	if (endCol > startCol && endRow > startRow)
	{
		// Decide which row to flush next
		if (nextFlushCol < startCol || nextFlushCol >= endCol)
		{
			nextFlushCol = startCol;	// start from the beginning
		}

		if (nextFlushCol == startCol)	// if we are starting from the beginning
		{
			startCol++; // flag this column as flushed because it will be soon
		}

		// Flush that column
		uint8_t startRowNum = startRow / 8;
		const uint8_t endRowNum = (endRow + 7) / 8;

		uint8_t *imgPtr = image + (startRowNum * NumCols + nextFlushCol);
		uint8_t i2cData[9];
		i2cData[0] = I2CLcdSendData;
		size_t numBytes = 1;
		while (startRowNum < endRowNum) {
			i2cData[numBytes++] = *imgPtr;
			imgPtr += NumCols;
			startRowNum++;
		}

		if (reprap.Debug(moduleDisplay))
		{
			debugPrintf("Send %uB @ %u %u (dirty %u,%u to %u,%u)\n", numBytes, startRowNum, nextFlushCol, startRow, startCol, endRow, endCol);
		}

		{
			MutexLocker Lock(Tasks::GetI2CMutex());
			setGraphicsAddress(startRow / 8, nextFlushCol);
//			delayMicroseconds(LcdCommandDelayMicros);
			Transfer(_ssd1309_addr, i2cData, numBytes, 0);
//			delayMicroseconds(LcdDataDelayMicros);
		}

		if (startCol != endCol)
		{
			++nextFlushCol;
			return true;
		}

		startRow = NumRows;
		startCol = NumCols;
		endCol = endRow = nextFlushCol = 0;
	}
	return false;
}

//bool Lcd1309::FlushSome() {
//	// See if there is anything to flush
//	if (endCol > startCol && endRow > startRow)
//	{
//		// Decide which row to flush next
//		if (nextFlushRow < startRow || nextFlushRow >= endRow)
//		{
//			nextFlushRow = startRow;	// start from the beginning
//		}
//
//		if (nextFlushRow == startRow)	// if we are starting form the beginning
//		{
//			startRow = (startRow & 0xF8) + 8;					// flag this row as flushed because it will be soon
//		}
//
//		// Flush that row
//		{
//			uint8_t startColNum = startCol;
//
//			setGraphicsAddress(nextFlushRow / 8, startColNum);
//
//			uint8_t *ptr = image + ((NumCols * (nextFlushRow / 8)) + startColNum);
//			uint8_t data[129];
//			data[0] = I2CLcdSendData;
//			size_t numBytes = 1;
//			while (startColNum < endCol)
//			{
//				data[numBytes++] = *ptr++;
//				++startColNum;
//			}
//			I2C::Transfer(_ssd1309_addr, data, numBytes, 0);
//		}
//
//		if (startRow < endRow)
//		{
//			++nextFlushRow;
//			return true;
//		}
//
//		startRow = NumRows;
//		startCol = NumCols;
//		endCol = endRow = nextFlushRow = 0;
//	}
//	return false;
//}

// Set the cursor position
void Lcd1309::SetCursor(PixelNumber r, PixelNumber c)
{
	row = r;
	column = c;
	lastCharColData = 0u;    // flag that we just set the cursor position, so no space before next character
	justSetCursor = true;
}

void Lcd1309::SetPadding(PixelNumber p)
{
	if (padding != p) {
		padding = p;
		if (!justSetCursor)
		{
			lastCharColData = 0xFFFF;				// force a space when switching padding
		}
	}
}

void Lcd1309::SetPixel(PixelNumber y, PixelNumber x, PixelMode mode)
{
	if (y < NumRows && x < rightMargin)
	{
		uint8_t * const p = image + (((y / 8) * NumCols) + x);
		const uint8_t mask = 0x80u >> (y % 8);
		const uint8_t oldVal = *p;
		uint8_t newVal;
		switch(mode)
		{
		case PixelMode::PixelClear:
			newVal = oldVal & ~mask;
			break;
		case PixelMode::PixelSet:
			newVal = oldVal | mask;
			break;
		case PixelMode::PixelFlip:
			newVal = oldVal ^ mask;
			break;
		default:
			newVal = oldVal;
			break;
		}

		if (newVal != oldVal)
		{
			*p = newVal;
			SetDirty(y, x);
		}
	}
}

bool Lcd1309::ReadPixel(PixelNumber x, PixelNumber y) const
{
	if (y < NumRows && x < NumCols)
	{
		const uint8_t * const p = image + (((y / 8) * NumCols) + x);
		return (*p & (0x80u >> (y % 8))) != 0;
	}
	return false;
}

// Set the address to write to. The row address is in 8-bit words, so it ranges from 0 to 7.
void Lcd1309::setGraphicsAddress(unsigned int r, unsigned int c)
{
	uint8_t positioningBytes[4] = {
			I2CLcdSendCommand,
			(uint8_t) (c & 0xF), 				 // 0x0X sets the lower nibble of the current column to X
			(uint8_t) (0x10 | ((c >> 4) & 0x7)), // 0x1Y sets the higher nibble to Y
			(uint8_t) (0xB0 | (r & 0x7)) 	 // 0xBZ sets the row address to Z
	};

	Transfer(_ssd1309_addr, positioningBytes, 4, 0);
}

#endif
