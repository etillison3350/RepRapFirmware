/*
 * Lcd.h
 *
 *  Created on: Mar 8, 2020
 *      Author: Ethan Tillison
 */

#ifndef LCD_12864_H_
#define LCD_12864_H_

#include "RepRapFirmware.h"

#if SUPPORT_12864_LCD

#include <Print.h>

// Enumeration for specifying drawing modes
enum class PixelMode : uint8_t
{
	PixelClear = 0,    // clear the pixel(s)
	PixelSet = 1,      // set the pixel(s)
	PixelFlip = 2      // invert the pixel(s)
};

// Struct for describing a font table, always held in PROGMEM
struct LcdFont
{
	const uint8_t *ptr;			// pointer to font table
	uint16_t startCharacter;	// Unicode code point of the first character in the font
	uint16_t endCharacter;		// Unicode code point of the last character in the font
	uint8_t height;				// row height in pixels - only this number of pixels will be fetched and drawn - maximum 16 in this version of the software
	uint8_t width;				// max character width in pixels (the font table contains this number of bytes or words per character, plus 1 for the active width)
	uint8_t numSpaces;			// number of space columns between characters before kerning
};

typedef uint8_t PixelNumber;
const PixelNumber NumRows = 64, NumCols = 128;

// Derive the LCD class from the Print class so that we can print stuff to it in alpha mode
class Lcd : public Print {
public:
	constexpr PixelNumber GetNumRows() const { return NumRows; }
	constexpr PixelNumber GetNumCols() const { return NumCols; }

	// Write a space
	virtual void WriteSpaces(PixelNumber numPixels) = 0;

	// Initialize the display. Call this in setup(). Also call setFont to select initial text font.
	virtual void Init() = 0;

	// Return the number of fonts
	virtual size_t GetNumFonts() const = 0;

	// Select the font to use for subsequent calls to write() in graphics mode
	virtual void SetFont(size_t newFont) = 0;

	// Get the current font height
	virtual PixelNumber GetFontHeight() const = 0;

	// Get the height of a specified font
	virtual PixelNumber GetFontHeight(size_t fontNumber) const = 0;

	// Select normal or inverted text (only works in graphics mode)
	virtual void TextInvert(bool b) = 0;

	// Clear the display and select non-inverted text.
	virtual void Clear(PixelNumber top = 0, PixelNumber left = 0, PixelNumber bottom = NumRows, PixelNumber right = NumCols) = 0;

	// Set the cursor position
	//  r = row, the number of pixels from the top of the display to the top of the character.
	//  c = column, is the number of pixels from the left hand edge of the display and the left hand edge of the character.
	virtual void SetCursor(PixelNumber r, PixelNumber c) = 0;

	// Get the cursor row. Useful after we have written some text.
	virtual PixelNumber GetRow() const = 0;

	// Get the cursor column. Useful after we have written some text.
	virtual PixelNumber GetColumn() const = 0;

	// Sets the vertical padding above/below text
	virtual void SetPadding(PixelNumber p) = 0;

	// Set the left margin. This is where the cursor goes to when we print newline.
	virtual void SetLeftMargin(PixelNumber c) = 0;

	// Set the right margin. In graphics mode, anything written will be truncated at the right margin. Defaults to the right hand edge of the display.
	virtual void SetRightMargin(PixelNumber r) = 0;

	// Clear a rectangle from the current position to the right margin (graphics mode only). The height of the rectangle is the height of the current font.
	virtual void ClearToMargin() = 0;

	// Flush the display buffer to the display. Data will not be committed to the display until this is called.
	virtual void FlushAll() = 0;

	// Flush just some data, returning true if this needs to be called again
	virtual bool FlushSome() = 0;

	// Set, clear or invert a pixel
	//  x = x-coordinate of the pixel, measured from left hand edge of the display
	//  y = y-coordinate of the pixel, measured down from the top of the display
	//  mode = whether we want to set, clear or invert the pixel
	virtual void SetPixel(PixelNumber y, PixelNumber x, PixelMode mode) = 0;

	// Read a pixel. Returns true if the pixel is set, false if it is clear.
	//  x = x-coordinate of the pixel, measured from left hand edge of the display
	//  y = y-coordinate of the pixel, measured down from the top of the display
	virtual bool ReadPixel(PixelNumber y, PixelNumber x) const = 0;

	// Draw a line.
	//  x0 = x-coordinate of one end of the line, measured from left hand edge of the display
	//  y0 = y-coordinate of one end of the line, measured down from the top of the display
	//  x1, y1 = coordinates of the other end od the line
	//  mode = whether we want to set, clear or invert each pixel
	virtual void Line(PixelNumber top, PixelNumber left, PixelNumber bottom, PixelNumber right, PixelMode mode);

	// Draw a circle
	//  x0 = x-coordinate of the centre, measured from left hand edge of the display
	//  y0 = y-coordinate of the centre, measured down from the top of the display
	//  radius = radius of the circle in pixels
	//  mode = whether we want to set, clear or invert each pixel
	virtual void Circle(PixelNumber row, PixelNumber col, PixelNumber radius, PixelMode mode);

	// Draw a bitmap
	//  x0 = x-coordinate of the top left, measured from left hand edge of the display. Currently, must be a multiple of 8.
	//  y0 = y-coordinate of the top left, measured down from the top of the display
	//  width = width of bitmap in pixels. Currently, must be a multiple of 8.
	//  rows = height of bitmap in pixels
	//  data = bitmap image, must be ((width/8) * rows) bytes long
	virtual void Bitmap(PixelNumber top, PixelNumber left, PixelNumber height, PixelNumber width, const uint8_t data[]) = 0;

	// Draw a bitmap row
	//  x0 = x-coordinate of the top left, measured from left hand edge of the display
	//  y0 = y-coordinate of the top left, measured down from the top of the display
	//  width = width of bitmap in pixels
	//  data = bitmap image, must be ((width + 7)/8) bytes long
	virtual void BitmapRow(PixelNumber top, PixelNumber left, PixelNumber width, const uint8_t data[], bool invert) = 0;
};

#endif

#endif /* SRC_DISPLAY_LCD_LCD_H_ */
