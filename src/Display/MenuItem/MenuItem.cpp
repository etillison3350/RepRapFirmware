/*
 * MenuItem.cpp
 *
 *  Created on: 7 May 2018
 *      Author: David
 */

#include "MenuItem.h"

#if SUPPORT_12864_LCD

#include "RepRap.h"
#include "Heating/Heat.h"
#include "Platform.h"
#include "GCodes/GCodes.h"
#include "Movement/Move.h"
#include "Display/Display.h"
#include "Tools/Tool.h"
#include "Networking/Network.h"
#include "PrintMonitor.h"

MenuItem::MenuItem(PixelNumber r, PixelNumber c, PixelNumber w, Alignment a, FontNumber fn, PixelNumber pad, Visibility vis)
	: row(r), column(c), width(w), height(0), align(a), fontNumber(fn), padding(pad), visCase(vis), itemChanged(true), highlighted(false), drawn(false), next(nullptr)
{
}

/*static*/ void MenuItem::AppendToList(MenuItem **root, MenuItem *item)
{
	while (*root != nullptr)
	{
		root = &((*root)->next);
	}
	item->next = nullptr;
	*root = item;
}

// Print the item at the correct place with the correct alignment
void MenuItem::PrintAligned(Lcd& lcd, PixelNumber tOffset, PixelNumber rightMargin)
{
	PixelNumber colsToSkip = 0;
	lcd.SetFont(fontNumber);
	if (align != LeftAlign)
	{
		// Draw the text offscreen to calculate the width
		lcd.SetCursor(lcd.GetNumRows(), column);
		lcd.SetPadding(padding);
		lcd.SetRightMargin(min<PixelNumber>(rightMargin, column + width));
		CorePrint(lcd);
		const PixelNumber w = lcd.GetColumn() - column;
		if (w < width)
		{
			colsToSkip = (align == RightAlign)
							? width - w - 1				// when right aligning, leave 1 pixel of space at the end
								: (width - w)/2;
		}
	}

	lcd.SetCursor(row - tOffset, column);
	lcd.SetRightMargin(min<PixelNumber>(rightMargin, column + width));
	lcd.TextInvert(highlighted);
	if (colsToSkip != 0)
	{
		lcd.ClearToMargin();
		lcd.SetCursor(row, column + colsToSkip);
	}
	CorePrint(lcd);
	if (align == LeftAlign)
	{
		lcd.ClearToMargin();
	}
	lcd.TextInvert(false);
}

bool MenuItem::IsVisible() const
{
	switch (visCase)
	{
	default:
	case 0:		return true;
	case 2:		return reprap.GetGCodes().IsReallyPrinting();
	case 3:		return !reprap.GetGCodes().IsReallyPrinting();
	case 4:		return reprap.GetPrintMonitor().IsPrinting();
	case 5:		return !reprap.GetPrintMonitor().IsPrinting();
	case 6:		return reprap.GetGCodes().IsPaused() || reprap.GetGCodes().IsPausing();
	case 7:		return reprap.GetGCodes().IsReallyPrinting() || reprap.GetGCodes().IsResuming();
	case 10:	return reprap.GetPlatform().GetMassStorage()->IsDriveMounted(0);
	case 11:	return !reprap.GetPlatform().GetMassStorage()->IsDriveMounted(0);
	case 20:	return reprap.GetCurrentOrDefaultTool()->HasTemperatureFault();
	case 28:	return reprap.GetHeat().GetStatus(reprap.GetHeat().GetBedHeater(0)) == Heat::HS_fault;
	}
}

// Erase this item if it is drawn but should not be visible
void MenuItem::EraseIfInvisible(Lcd& lcd, PixelNumber tOffset)
{
	if (drawn && !IsVisible())
	{
		lcd.Clear(row - tOffset, column, row + height, column + width);
		drawn = false;
	}
}

#endif

// End
