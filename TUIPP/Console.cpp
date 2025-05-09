#include "Console.hpp"
#include "assert.h"

Console::Console(const std::string& title, uint16_t width, uint16_t height, uint16_t pixelWidth, uint16_t pixelHeight) :
	Window(title, Window::Type::Console, width* pixelWidth, height* pixelHeight),
	m_nPixelWidth(pixelWidth),
	m_nPixelHeight(pixelHeight),
	m_nWidth(width),
	m_nHeight(height),
	m_targetedComponent(0xffffffff)
{
	if (this->IsOpen())
	{
		this->m_hInConsole = GetStdHandle(STD_INPUT_HANDLE);
		this->m_hOutConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		//this->SetSize(width * pixelWidth, height * pixelHeight);
		
		SMALL_RECT rect = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(this->GetOutputHandle(), TRUE, &rect);

		if (!SetConsoleScreenBufferSize(this->GetOutputHandle(), {(short)width, (short)height}))
			MessageBoxA(NULL, "Couldn't set the console screen buffer size!", "Error", MB_ICONERROR | MB_OK);

		SetConsoleActiveScreenBuffer(this->GetOutputHandle());

		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = pixelWidth;
		cfi.dwFontSize.Y = pixelHeight;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;

		wcscpy_s(cfi.FaceName, L"Consolas");
		SetCurrentConsoleFontEx(this->GetOutputHandle(), FALSE, &cfi);

		this->m_ViewRect.Left = 0;
		this->m_ViewRect.Top = 0;
		this->m_ViewRect.Right = width - 1;
		this->m_ViewRect.Bottom = height - 1;
		
		if (!SetConsoleWindowInfo(this->GetOutputHandle(), TRUE, &this->m_ViewRect))
		{
			this->SetSize(width * pixelWidth, height*pixelHeight);
		}
		
		SetConsoleMode(this->GetInputHandle(), ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

		this->re.buffer.resize(this->m_nWidth * this->m_nHeight, CHAR_INFO());
		this->re.hConsole = this->GetOutputHandle();
		this->re.viewRect = this->GetViewRect();
		this->InitEventProcessor(this->GetInputHandle());
	}
}

Console::~Console()
{
	
}

HANDLE Console::GetOutputHandle() const
{
	return this->m_hOutConsole;
}

HANDLE Console::GetInputHandle() const
{
	return this->m_hInConsole;
}

COORD Console::GetConsoleSize() const
{
	assert(m_nPixelWidth > 0 && m_nPixelHeight > 0);
	return { short(this->GetSize().X / this->m_nPixelWidth), short(this->GetSize().Y / this->m_nPixelHeight) };
}

SMALL_RECT Console::GetViewRect() const
{
	return this->m_ViewRect;
}

bool Console::InsertComponent(const std::shared_ptr<Component>& component)
{
	m_components.push_back(component);
	component->OnInit();
	return true;
}

void Console::SetConsoleSize(uint16_t width, uint16_t height, bool updateBuffer)
{
	this->SetSize(width * this->m_nPixelWidth, height * this->m_nPixelWidth);

	SetConsoleScreenBufferSize(this->GetOutputHandle(), { (short)width, (short)height });

	this->re.viewRect.Right = width;
	this->re.viewRect.Bottom = height;

	SetConsoleWindowInfo(this->GetOutputHandle(), TRUE, &this->m_ViewRect);
	if(updateBuffer)
	{
		std::vector<CHAR_INFO> newBuffer;
		newBuffer.reserve(width * height);
		std::memcpy(newBuffer.data(), this->re.buffer.data(), width * height * sizeof CHAR_INFO);

		this->re.buffer.clear();
		this->re.buffer.shrink_to_fit();

		this->re.buffer.reserve(newBuffer.size());
		this->re.buffer.insert(this->re.buffer.begin(), newBuffer.begin(), newBuffer.end());
	}
}

void Console::Display()
{
	this->ProcessEvents();
	for (int i = 0; i < this->m_components.size(); i++)
	{
		auto& component = this->m_components[i];
		component->Update(this);

		if (component->m_type == Component::Type::Panel && component->m_targeted)
		{
			this->m_targetedComponent = i;
		}
		component->Render(this);
	}
	if (this->m_targetedComponent != 0xffffffff)
	{

		this->m_components[this->m_targetedComponent]->Update(this);
		this->m_components[this->m_targetedComponent]->Render(this);
	}
	//this->SetPixel(this->GetMousePos().X, this->GetMousePos().Y, 0x2588, 0x11);
	SMALL_RECT rect = this->GetViewRect();
	WriteConsoleOutputW(this->GetOutputHandle(), re.buffer.data(), {short(this->re.viewRect.Right), short(this->re.viewRect.Bottom)}, {}, &rect);
}

