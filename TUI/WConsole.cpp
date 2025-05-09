#include "WConsole.hpp"

WConsole::WConsole(const std::string& title, uint16_t width, uint16_t height, uint16_t pixelWidth, uint16_t pixelHeight) :
	Window(title, Type::WinConsole, width * pixelWidth, height * pixelHeight) {

}

WConsole::~WConsole() {

}
