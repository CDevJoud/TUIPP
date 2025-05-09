#include "RenderTarget.hpp"

void RenderTarget::FlushTo(RenderTarget* out, SMALL_RECT rect)
{
	COORD p1 = { rect.Left, rect.Top };
	COORD p2 = { p1.X + rect.Right, p1.Y + rect.Bottom};
	for (int x = p1.X; x < p2.X; x++)
	{
		for (int y = p1.Y; y < p2.Y; y++)
		{
			uint16_t px = (x - p1.X);
			uint16_t py = (y - p1.Y);

			auto pixel = this->GetPixelAt(px, py);
			out->SetPixel(x, y, pixel.Char.UnicodeChar, pixel.Attributes);
		}
	}
}

RenderTarget::~RenderTarget()
{
	this->re.buffer.clear();
	this->re.buffer.shrink_to_fit();
}

void RenderTarget::SetPixel(int16_t x, int16_t y, uint16_t c, uint8_t color)
{
	if (x >= re.viewRect.Left && x < re.viewRect.Right && y >= re.viewRect.Top && y < re.viewRect.Bottom)
	{
		this->re.buffer[y * re.viewRect.Right + x].Char.UnicodeChar = c;
		this->re.buffer[y * re.viewRect.Right + x].Attributes = color;
	}
}

void RenderTarget::CalculateClipOn(int16_t& x, int16_t& y)
{
	if (x < 0) x = 0;
	if (x > this->re.viewRect.Right) x = this->re.viewRect.Right;
	if (y < 0) y = 0;
	if (y > this->re.viewRect.Bottom) y = this->re.viewRect.Bottom;
}

void RenderTarget::Fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t c, uint8_t color)
{
	this->CalculateClipOn(x1, y1);
	this->CalculateClipOn(x2, y2);
	for (uint16_t h = y1; h < y2; h++)
		for (uint16_t w = x1; w < x2; w++)
			this->SetPixel(w, h, c, color);
}

void RenderTarget::ClearScreen(uint16_t c, uint8_t color)
{
	this->Fill(0, 0, this->re.viewRect.Right, this->re.viewRect.Bottom, c, color);
}

void RenderTarget::RenderLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t c, uint8_t color)
{
	INT x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
	dx = x2 - x1; dy = y2 - y1;
	dx1 = abs(dx); dy1 = abs(dy);
	px = 2 * dy1 - dx1;	py = 2 * dx1 - dy1;
	if (dy1 <= dx1)
	{
		if (dx >= 0)
		{
			x = x1; y = y1; xe = x2;
		}
		else
		{
			x = x2; y = y2; xe = x1;
		}

		this->SetPixel(x,y, c, color);

		for (i = 0; x < xe; i++)
		{
			x = x + 1;
			if (px < 0)
				px = px + 2 * dy1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
				px = px + 2 * (dy1 - dx1);
			}
			this->SetPixel(x, y, c, color);
		}
	}
	else
	{
		if (dy >= 0)
		{
			x = x1; y = y1; ye = y2;
		}
		else
		{
			x = x2; y = y2; ye = y1;
		}

		this->SetPixel(x, y, c, color);

		for (i = 0; y < ye; i++)
		{
			y = y + 1;
			if (py <= 0)
				py = py + 2 * dx1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
				py = py + 2 * (dx1 - dy1);
			}
			this->SetPixel(x, y, c, color);
		}
	}
}

void RenderTarget::RenderText(uint16_t x, uint16_t y, const std::string& str, uint8_t color)
{
	for (int i = 0; i < str.length(); i++)
	{
		this->SetPixel(x + i, y, str[i], color);
	}
}

bool RenderTarget::CheckInBoundaries(COORD pos, SMALL_RECT rect)
{
	return (pos.X >= rect.Left && pos.X <= rect.Right &&
		pos.Y >= rect.Top && pos.Y <= rect.Bottom);
}

SMALL_RECT RenderTarget::GetViewRect() const
{
	return this->re.viewRect;
}

CHAR_INFO RenderTarget::GetPixelAt(uint16_t x, uint16_t y) const
{
	if (x >= re.viewRect.Left && x < re.viewRect.Right && y >= re.viewRect.Top && y < re.viewRect.Bottom)
	{
		return re.buffer[y * re.viewRect.Right + x];
	}
	else
		return CHAR_INFO();
}

PCHAR_INFO RenderTarget::GetPixelBuffer() const
{
	return const_cast<PCHAR_INFO>(this->re.buffer.data());
}

void RenderTarget::RenderTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t c, uint8_t color)
{
	this->RenderLine(x1, y1, x2, y2, c, color);
	this->RenderLine(x2, y2, x3, y3, c, color);
	this->RenderLine(x3, y3, x1, y1, c, color);
}

void RenderTarget::RenderCircle(int16_t xx, int16_t yy, int32_t radius, uint16_t c, uint8_t color, std::function<void(int32_t& x, int32_t& y, int32_t& p)> func)
{
	//Flags will be added later!
	int xc = xx;
	int yc = yy;
	INT x = 0;
	INT y = radius;
	INT p = 3 - 2 * radius;

	if (!radius) return;
	while (y >= x)
	{
		this->SetPixel(xc - x, yc - y, c, color);
		this->SetPixel(xc - y, yc - x, c, color);
		this->SetPixel(xc + y, yc - x, c, color);
		this->SetPixel(xc + x, yc - y, c, color);
		this->SetPixel(xc - x, yc + y, c, color);
		this->SetPixel(xc - y, yc + x, c, color);
		this->SetPixel(xc + y, yc + x, c, color);
		this->SetPixel(xc + x, yc + y, c, color);
		if (func != nullptr)
			func(x, y, p);
		else
			p += (p < 0) ? 4 * x++ + (6) : 4 * (x++ - y--) + 10;
	}
}

VOID RenderTarget::RasterizeTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t c, uint8_t color)
{
	auto SWAP = [](int16_t& x, int16_t& y) { INT t = x; x = y; y = t; };
	auto SWAP32 = [](INT& x, INT& y) { INT t = x; x = y; y = t; };
	auto drawline = [&](INT sx, INT ex, INT ny) { for (INT i = sx; i <= ex; i++) SetPixel(i, ny, c, color); };

	INT t1x, t2x, y, minx, maxx, t1xp, t2xp;
	bool changed1 = false;
	bool changed2 = false;
	INT signx1, signx2, dx1, dy1, dx2, dy2;
	INT e1, e2;
	// Sort vertices
	if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
	if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
	if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

	t1x = t2x = x1; y = y1;   // Starting poINTs
	dx1 = (INT)(x2 - x1); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (INT)(y2 - y1);

	dx2 = (INT)(x3 - x1); if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
	else signx2 = 1;
	dy2 = (INT)(y3 - y1);

	if (dy1 > dx1) {   // swap values
		SWAP32(dx1, dy1);
		changed1 = TRUE;
	}
	if (dy2 > dx2) {   // swap values
		SWAP32(dy2, dx2);
		changed2 = TRUE;
	}

	e2 = (INT)(dx2 >> 1);
	// Flat top, just process the second half
	if (y1 == y2) goto next;
	e1 = (INT)(dx1 >> 1);

	for (INT i = 0; i < dx1;) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			i++;
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) t1xp = signx1;//t1x += signx1;
				else          goto next1;
			}
			if (changed1) break;
			else t1x += signx1;
		}
		// Move line
	next1:
		// process second line until y value is about to change
		while (1) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;//t2x += signx2;
				else          goto next2;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next2:
		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);    // Draw line from min to max poINTs found on the y
		// Now increase y
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y == y2) break;

	}
next:
	// Second half
	dx1 = (INT)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (INT)(y3 - y2);
	t1x = x2;

	if (dy1 > dx1) {   // swap values
		SWAP32(dy1, dx1);
		changed1 = TRUE;
	}
	else changed1 = FALSE;

	e1 = (INT)(dx1 >> 1);

	for (INT i = 0; i <= dx1; i++) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) { t1xp = signx1; break; }//t1x += signx1;
				else          goto next3;
			}
			if (changed1) break;
			else   	   	  t1x += signx1;
			if (i < dx1) i++;
		}
	next3:
		// process second line until y value is about to change
		while (t2x != x3) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;
				else          goto next4;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next4:

		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y > y3) return;
	}
}