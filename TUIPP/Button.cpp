#include "Button.hpp"

void Button::SetUpFrame(RenderTarget* rt, SMALL_RECT rect, uint8_t color)
{
	SMALL_RECT;
	uint16_t x = rect.Left;
	uint16_t y = rect.Top;
	uint16_t width = rect.Right;
	uint16_t height = rect.Bottom;
	// Top Horizontal Line
	rt->RenderLine(x - 1, y - 1, x + width, y - 1, 0x2500, color);

	// Left Vertical Line
	rt->RenderLine(x - 1, y - 1, x - 1, y + height - 1, 0x2502, color);

	// Bottom Horizontal Line
	rt->RenderLine(x - 1, y + height, x + width, y + height, 0x2500, color);

	// Right Vertical Line
	rt->RenderLine(x + width, y - 1, x + width, y + height - 1, 0x2502, color);

	// Corner Characters
	rt->SetPixel(x - 1, y - 1, 0x256D, color);           // Top-Left
	rt->SetPixel(x + width, y - 1, 0x256E, color);       // Top-Right
	rt->SetPixel(x - 1, y + height, 0x2570, color);      // Bottom-Left
	rt->SetPixel(x + width, y + height, 0x256F, color);  // Bottom-Right
}


Button::Button(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc) :
	Component(Component::Type::Button, &this->re, title, fc),
	m_title(title),
	m_isHovering(false),
	m_hoveringColor(0x00),
	m_withFrame(false)
{
	if (width == 0xffff) {
		this->m_nWidth = this->m_title.length() + 2;
	}
	else
		this->m_nWidth = width;
	if (height == 0xffff) {
		this->m_nHeight = 1;
	}
	else
		this->m_nHeight = height;
	this->Create(this->m_title, this->m_nWidth, this->m_nHeight);
}

Button::Button(const std::string& title, uint16_t width, uint16_t height, bool withFrame, FunctionContainer fc) :
	Component(Component::Type::Button, &this->re, title, fc),
	m_title(title),
	m_isHovering(false),
	m_hoveringColor(0x00),
	m_withFrame(withFrame)
{
	if (width == 0xffff)
	{
		this->m_nWidth = this->m_title.length() + 2;
	}
	else
		this->m_nWidth = width;
	if (height == 0xffff)
	{
		this->m_nHeight = 1;
	}
	else
		this->m_nHeight = height;
	this->Create(this->m_title, this->m_nWidth, this->m_nHeight);
}

void Button::Create(const std::string& title, uint16_t width, uint16_t height)
{
	this->m_nWidth = width;
	this->m_nHeight = height;
	this->m_title = title;
	this->re.buffer.resize(width * height);
	this->re.viewRect = { 0, 0, short(width), short(height) };
}

std::shared_ptr<Button> Button::CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc)
{
	return std::make_shared<Button>(title, width, height, fc);
}

std::shared_ptr<Button> Button::CreateInstance(const std::string& title, uint16_t width, uint16_t height, bool withFrame, FunctionContainer fc)
{
	return std::make_shared<Button>(title, width, height, withFrame, fc);
}

void Button::Update(EventProcessor* ep)
{
	if (RenderTarget::CheckInBoundaries(ep->GetMousePos(), {
		Component::GetPosition().X,
		Component::GetPosition().Y,
		short(Component::GetPosition().X + Component::GetSize().X - 1),
		short(Component::GetPosition().Y + Component::GetSize().Y - 1)
		}))
	{
		Button::m_hoveringColor = ~this->m_color;
		Button::m_isHovering = true;
		if (Component::GetFunctionContainer().Find("OnClicked"))
		{
			if (ep->Mouse(EventProcessor::MouseType::Left).bStrokePressed)
				Component::GetFunctionContainer().CallFunction<void, EventProcessor::MouseType>("OnClicked", EventProcessor::MouseType::Left);
			if (ep->Mouse(EventProcessor::MouseType::Right).bStrokePressed)
				Component::GetFunctionContainer().CallFunction<void, EventProcessor::MouseType>("OnClicked", EventProcessor::MouseType::Right);
			if (ep->Mouse(EventProcessor::MouseType::Middle).bStrokePressed)
				Component::GetFunctionContainer().CallFunction<void, EventProcessor::MouseType>("OnClicked", EventProcessor::MouseType::Middle);
		}
	}
	else
		Button::m_isHovering = false;
}

uint8_t GetTextBackColor(uint8_t color)
{
	return color & 0xF0;
}

void Button::Render(RenderTarget* out)
{
	uint8_t backColor = 0x00;
	if (Button::m_isHovering)
		backColor = this->m_hoveringColor & 0xF0;
	else
		backColor = this->m_hoveringColor & 0x0F;

	if (backColor == 0xF0)
		m_frontColor = 0x00;

	uint8_t bColor = backColor | m_frontColor;
	
	RenderTarget::Fill(0, 0,
			  this->m_nWidth,
			  this->m_nHeight,
			  0x0000, backColor);
	if (Button::m_withFrame)
		Button::SetUpFrame(out, { Component::GetPosition().X, Component::GetPosition().Y, Component::GetSize().X, Component::GetSize().Y }, 0x0f);
	RenderTarget::RenderText((Component::GetSize().X / 2) - (Button::GetTitle().length() / 2), Component::GetSize().Y / 2, this->m_title, bColor);
	RenderTarget::FlushTo(out, { this->GetPosition().X, this->GetPosition().Y, this->GetSize().X, this->GetSize().Y });
}

void Button::OnInit()
{
	if (Component::GetFunctionContainer().Find("OnInit"))
		Component::GetFunctionContainer().CallFunction<void>("OnInit");
}

void Button::WithFrame(bool value)
{
	Button::m_withFrame = value;
}

std::string Button::GetTitle() const
{
	return Button::m_title;
}

void Button::SetTextColor(const uint8_t color)
{
	this->m_frontColor = color;
}
