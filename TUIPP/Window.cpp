#define NOMINMAX
#include "Window.hpp"


BOOL WINAPI ConsoleHandler(DWORD event)
{
	return TRUE;
}

Window::Window(const std::string& title, const Type& type, const uint16_t width, const uint16_t height) : 
	m_type(type),
	m_hwnd(nullptr)
{
	if (this->m_type == Type::Console)
	{
		if (this->InitConsoleWindow())
		{
			this->m_bIsOpen = (bool)this->m_hwnd;
			this->SetSize(width, height);
			RECT wndRect, deskRect;
			
			GetWindowRect(this->m_hwnd, &wndRect);
			GetWindowRect(GetDesktopWindow(), &deskRect);

			// Calculate the window width and height
			int wndWidth = wndRect.right - wndRect.left;
			int wndHeight = wndRect.bottom - wndRect.top;

			// Calculate the desktop width and height
			int deskWidth = deskRect.right - deskRect.left;
			int deskHeight = deskRect.bottom - deskRect.top;

			// Calculate new top-left position for the window
			int x = (deskWidth - wndWidth) / 2;
			int y = (deskHeight - wndHeight) / 2;

			this->SetPosition(x, y);

			this->SetTitle(title);
			
			SetConsoleCtrlHandler(ConsoleHandler, TRUE);
		}
	}
	/*else if(this->m_type == Type::WinConsole) {
		this->m_window = new sf::RenderWindow(sf::VideoMode(width, height), title);
		this->m_hwnd = this->m_window->getSystemHandle();
	}*/
}

Window::~Window()
{
	switch (this->m_type)
	{
	default:
		break;

	case Type::Console:
		DestroyWindow(this->m_hwnd);
		FreeConsole();
		break;
	}
}

bool Window::IsOpen() const
{
	return this->m_bIsOpen;
}

HWND Window::GetSystemHandle() const
{
	return this->m_hwnd;
}

void Window::SetPosition(const uint16_t x, const uint16_t y)
{
	RECT wndRect;
	GetWindowRect(this->m_hwnd, &wndRect);
	MoveWindow(this->m_hwnd, x, y, wndRect.right, wndRect.bottom, TRUE);
}

void Window::SetSize(const uint16_t width, const uint16_t height)
{
	RECT WndRect;
	GetWindowRect(this->m_hwnd, &WndRect);
	MoveWindow(this->m_hwnd, WndRect.left, WndRect.top, width, height, TRUE);
}

COORD Window::GetSize() const
{
	RECT WndRect;
	GetWindowRect(this->m_hwnd, &WndRect);
	return { (short)WndRect.right, (short)WndRect.bottom };
}

COORD Window::GetPosition() const
{
	RECT wndRect;
	GetWindowRect(this->m_hwnd, &wndRect);
	return { (short)wndRect.left, (short)wndRect.top };
}

void Window::Close(bool forceClose)
{
	if (forceClose)
	{
		SendMessageA(this->m_hwnd, WM_CLOSE, 0, 0);
	}
	this->m_bIsOpen = false;
}

std::string Window::GetTitle() const
{
	CHAR buffer[MAX_PATH]{};
	GetWindowTextA(this->m_hwnd, buffer, MAX_PATH);
	return buffer;
}

void Window::SetTitle(const std::string& title)
{
	SetWindowTextA(this->m_hwnd, title.c_str());
}

bool Window::InitConsoleWindow()
{
	this->m_hwnd = GetConsoleWindow();
	if (!this->m_hwnd)
	{
		AllocConsole();
		this->m_hwnd = GetConsoleWindow();
		if (!this->m_hwnd)
		{
			MessageBoxA(nullptr, "Could not init Console Window", "Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}
	return true;
}
