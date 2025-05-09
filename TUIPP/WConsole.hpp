#pragma once
#include "Window.hpp"
#include "EventProcessor.hpp"
#include "Panel.hpp"
#include "Button.hpp"
#include <memory>

class WConsole : public Window{
public:
	WConsole(const std::string& title, uint16_t width, uint16_t height, uint16_t pixelWidth, uint16_t pixelHeight);
	~WConsole();
};

