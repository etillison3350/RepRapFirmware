/*
 * ButtonMenuItem.cpp
 *
 *  Created on: Feb 21, 2021
 */

#include "MenuItem.h"

#if SUPPORT_12864_LCD

#include "RepRap.h"

ButtonMenuItem::ButtonMenuItem(PixelNumber r, PixelNumber c, PixelNumber w, FontNumber fn, PixelNumber pad, Visibility vis, const char* t, const char* cmd, char const* acFile)
	: MenuItem(r, c, w, CentreAlign, pad, fn, vis), text(t), command(cmd), m_acFile(acFile)
{
}

void ButtonMenuItem::CorePrint(Lcd& lcd)
{
	lcd.WriteSpaces(1);				// space at start in case highlighted
	lcd.print(text);
	lcd.WriteSpaces(1);				// space at end to allow for highlighting
}

void ButtonMenuItem::Draw(Lcd& lcd, PixelNumber rightMargin, bool highlight, PixelNumber tOffset)
{
	if (IsVisible() && (itemChanged || !drawn || highlight != highlighted) && column < lcd.GetNumCols())
	{
		highlighted = highlight;
		PrintAligned(lcd, tOffset, rightMargin);
		itemChanged = false;
		drawn = true;
	}
}

void ButtonMenuItem::UpdateWidthAndHeight(Lcd& lcd)
{
	if (width == 0)
	{
		lcd.SetFont(fontNumber);
		lcd.SetCursor(lcd.GetNumRows(), 0);
		lcd.SetRightMargin(lcd.GetNumCols());
		lcd.TextInvert(false);
		CorePrint(lcd);
		width = lcd.GetColumn();
	}
	if (height == 0)
	{
		lcd.SetFont(fontNumber);
		height = lcd.GetFontHeight();
	}
}

// TODO WS1: if we overflow the command or directory string, we should probably offer a return value that tells the caller to do nothing...
bool ButtonMenuItem::Select(const StringRef& cmd)
{
	const int nReplacementIndex = StringContains(command, "#0");
	if (-1 != nReplacementIndex)
	{
		cmd.copy(command, nReplacementIndex);
		cmd.cat(m_acFile);
	}
	else
	{
		cmd.copy(command);
		if (StringEqualsIgnoreCase(command, "menu") && strlen(m_acFile) != 0)
		{
			// For backwards compatibility, if 'menu' is used without any parameters, use the L parameter as the name of the menu file
			cmd.cat(' ');
			cmd.cat(m_acFile);
		}
	}

	return true;
}

PixelNumber ButtonMenuItem::GetVisibilityRowOffset(PixelNumber tCurrentOffset, PixelNumber fontHeight) const
{
	PixelNumber tOffsetRequest = tCurrentOffset;

	// Are we off the bottom of the visible window?
	if (64 + tCurrentOffset <= row + fontHeight + 1)
	{
		tOffsetRequest = row - 3;
	}

	// Should we move back up?
	if (row < tCurrentOffset + 3)
	{
		tOffsetRequest = (row > 3) ? row - 3 : 0;
	}

	return tOffsetRequest;
}

#endif
