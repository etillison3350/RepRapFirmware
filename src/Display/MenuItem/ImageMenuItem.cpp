
#include "MenuItem.h"

#if SUPPORT_12864_LCD

#include "RepRap.h"
#include "Platform.h"

// Image menu item members
// The image file format is:
// Byte 0 = number of columns
// Byte 1 = number of rows
// Remaining bytes = data, 1 row at a time. If the number of columns is not a multiple of 8 then the data for each row is padded to a multiple of 8 bits.
ImageMenuItem::ImageMenuItem(PixelNumber r, PixelNumber c, Visibility vis, const char *pFileName)
	: MenuItem(r, c, 0, 0, 0, 0, vis)
{
	fileName.copy(pFileName);
}

void ImageMenuItem::Draw(Lcd& lcd, PixelNumber rightMargin, bool highlight, PixelNumber tOffset)
{
	if (IsVisible() && (!drawn || itemChanged || highlight != highlighted))
	{
		FileStore * const fs = reprap.GetPlatform().OpenFile(MENU_DIR, fileName.c_str(), OpenMode::read);
		if (fs != nullptr)
		{
			uint8_t widthAndHeight[2];
			if (fs->Read(widthAndHeight, 2) == 2)
			{
				const PixelNumber cols = widthAndHeight[0];
				const PixelNumber rows = widthAndHeight[1];
				if (cols != 0 && cols <= lcd.GetNumCols() && rows != 0)
				{
					const size_t bytesPerRow = (cols + 7)/8;
					for (PixelNumber irow = 0; irow < rows; ++irow)
					{
						uint8_t buffer[lcd.GetNumCols()/8];
						if (fs->Read(buffer, bytesPerRow) != (int)bytesPerRow)
						{
							break;
						}
						lcd.BitmapRow(row - tOffset + irow, column,  cols, buffer, highlight);
					}
				}
			}
			fs->Close();
		}
		itemChanged = false;
		drawn = true;
		highlighted = highlight;
	}
}

void ImageMenuItem::UpdateWidthAndHeight(Lcd& lcd)
{
	if (width == 0 || height == 0)
	{
		FileStore * const fs = reprap.GetPlatform().OpenFile(MENU_DIR, fileName.c_str(), OpenMode::read);
		if (fs != nullptr)
		{
			uint8_t w[2];
			fs->Read(w, 2);			// read the number of columns
			fs->Close();
			width = w[0];
			height = w[1];
		}
	}
}

#endif
