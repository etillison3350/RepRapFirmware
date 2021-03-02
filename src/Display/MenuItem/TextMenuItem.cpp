/*
 * ButtonMenuItem.cpp
 *
 *  Created on: Feb 21, 2021
 */

#include "MenuItem.h"

#if SUPPORT_12864_LCD

#include "RepRap.h"

TextMenuItem::TextMenuItem(PixelNumber r, PixelNumber c, PixelNumber w, Alignment a, FontNumber fn, PixelNumber pad, Visibility vis, const char* t)
	: MenuItem(r, c, w, a, fn, pad, vis), text(t)
{
}

void TextMenuItem::CorePrint(Lcd& lcd)
{
	lcd.print(text);
}

void TextMenuItem::Draw(Lcd& lcd, PixelNumber rightMargin, bool highlight, PixelNumber tOffset)
{
	// We ignore the 'highlight' parameter because text items are not selectable
	if (IsVisible() && (!drawn || itemChanged))
	{
		PrintAligned(lcd, tOffset, rightMargin);
		itemChanged = false;
		drawn = true;
	}
}

void TextMenuItem::UpdateWidthAndHeight(Lcd& lcd)
{
	if (width == 0)
	{
		lcd.SetFont(fontNumber);
		lcd.SetCursor(lcd.GetNumRows(), 0);
		lcd.SetRightMargin(lcd.GetNumCols());
		lcd.TextInvert(false);
		lcd.print(text);
		width = lcd.GetColumn();
		if (align == LeftAlign)
		{
			++width;			// add a space column after left-aligned text with no explicit width, so that the next item can follow immediately
		}
	}
	if (height == 0)
	{
		lcd.SetFont(fontNumber);
		height = lcd.GetFontHeight();
	}
}

#endif
