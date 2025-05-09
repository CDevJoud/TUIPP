#pragma once
#include "Windows.h"
#include <cstdint>
#include <vector>
#include <string>
#include <functional>

struct RenderElement
{
	HANDLE hConsole;
	std::vector<CHAR_INFO> buffer;
	SMALL_RECT viewRect;
};


class RenderTarget
{
public:
	void FlushTo(RenderTarget* out, SMALL_RECT rect);
	~RenderTarget();

	void SetPixel(int16_t x, int16_t y, uint16_t c, uint8_t color);
	void Fill(int16_t x1, int16_t y1, int16_t x2, int16_t, uint16_t c, uint8_t color);
	void ClearScreen(uint16_t c = 0x2588, uint8_t color = 0x00);
	
	void RenderLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t c, uint8_t color);
	void RenderText(uint16_t x, uint16_t y, const std::string& str, uint8_t color);
	void RasterizeTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t c, uint8_t color);
	void RenderTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t c, uint8_t color);
	void RenderCircle(int16_t x, int16_t y, int32_t radius, uint16_t c, uint8_t color, std::function<void(int32_t& x, int32_t& y, int32_t& p)> func = nullptr);
	
	void CalculateClipOn(int16_t& x, int16_t& y);
	bool CheckInBoundaries(COORD pos, SMALL_RECT rect);
	SMALL_RECT GetViewRect() const;
protected:
	RenderElement re;
	friend class Console;
	CHAR_INFO GetPixelAt(uint16_t x, uint16_t y) const;
	PCHAR_INFO GetPixelBuffer() const;
private:
	
};

