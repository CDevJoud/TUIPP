#pragma once
#include <Windows.h>
#include <string>

#define as =
#define TUI_STARTUP(_App) int main() {_App app; return app.main();}
#ifdef UNICODE
#define TUIWIN32_STARTUP(_App) INT APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ INT) {_App app; INT nExitCode = app.main(); ExitProcess(nExitCode);}
#endif
class TUIStartup {
public:
	virtual int main() = 0;
};

class Window
{
public:
	enum class Type
	{
		NONE = 0,
		WinConsole,
		Console,
		UWP
	};
	Window(const std::string& title, const Type& type, const uint16_t width, const uint16_t height);
	~Window();
	bool IsOpen() const;
	HWND GetSystemHandle() const;
	void SetPosition(const uint16_t x, const uint16_t y);
	void SetSize(const uint16_t width, const uint16_t height);

	COORD GetSize() const;
	COORD GetPosition() const;

	void Close(bool forceClose = false);

	std::string GetTitle() const;
	void SetTitle(const std::string& title);
private:
	bool InitConsoleWindow();
	bool m_bIsOpen;
	HWND m_hwnd;
	Type m_type;
};

