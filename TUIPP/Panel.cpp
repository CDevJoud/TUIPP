#include "Panel.hpp"
#include "Button.hpp"

SMALL_RECT Panel::GetRect() const
{
	return {
		Component::GetPosition().X,
		Component::GetPosition().Y,
		Component::GetSize().X,
		Component::GetSize().Y
	};
}

Panel& Panel::SetResizability(bool b) {
	this->isResizable = b;
	return *this;
}

bool Panel::IsResizable() const {
	return this->isResizable;
}

Panel& Panel::SetMovability(bool b) {
	this->isMovable = b;
	return *this;
}

bool Panel::IsMovable() const {
	return this->isMovable;
}

bool Panel::IsHovering() const {
	return this->isHovering;
}

Panel& Panel::SetTitleAlignment(TitleAlignment alignment) {
	this->titleAlignment = alignment;
	return *this;

}

Panel& Panel::SetTitleAlignmentOffset(int16_t offset) {
	this->titleAlignmentOffset = offset;
	return *this;
}

Panel& Panel::SetBorderColor(uint8_t color) {
	this->nBorderColor = color;
	return *this;
}

RenderTarget& Panel::GetRenderTarget()
{
	// TODO: insert return statement here
	return *this;
}

Component& Panel::GetComponent()
{
	// TODO: insert return statement here
	return *this;
}

void Panel::SetUpFrame(RenderTarget* rt, SMALL_RECT rect, uint8_t color)
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


Panel::Panel() :
	m_title(""),
	Component(Component::Type::Panel, &this->re, "NONE!"),
	titleAlignment(TitleAlignment::Center),
	titleAlignmentOffset(0),
	nBorderColor(0x0f)
{

}

Panel::Panel(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc) :
	m_title(title),
	Component(Component::Type::Panel, &this->re, title, fc),
	titleAlignment(TitleAlignment::Center),
	titleAlignmentOffset(0),
	nBorderColor(0x0f)
{
	this->Create(title, width, height);
}

std::shared_ptr<Panel> Panel::CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc)
{
	auto component = std::make_shared<Panel>(title, width, height, fc);
	if (component->re.buffer.empty()) {
		return nullptr;
	}
	component->SetPosition(1, 1);
	return component;
}

bool Panel::Create(const std::string& title, uint16_t width, uint16_t height)
{
	if (!RenderTarget::CheckInBoundaries({(short)width, (short)height}, {0, 0, 512, 512})) {
		return false;
	}
	else {
		this->m_nWidth = width;
		this->m_nHeight = height;
		this->m_title = title;
		this->re.buffer.resize(width * height);
		this->re.viewRect = { 0, 0, (short)width, (short)height };
		return true;
	}
}

bool Panel::InsertComponent(const std::shared_ptr<Component>& component)
{
	auto cType = component->GetType();
	
	switch (cType) {
	case Component::Type::Panel:
		m_components.push_back(component);
		break;
	case Component::Type::Button:
	{
		auto lastComponent = (this->m_components.empty()) ? std::shared_ptr<Component>() : *(std::prev(this->m_components.end()));
		if (lastComponent == nullptr) {
			component->SetPosition(0, 0);
			this->m_components.push_back(component);
			component->OnInit();
			return true;
		}
		else {
			if ((lastComponent->GetType() & Component::Type::Panel) != Component::Type::NONE) {
				component->SetPosition(0, lastComponent->GetPosition().Y + 2);
			}
			else {
				component->SetPosition(0, lastComponent->GetPosition().Y + 1);

			}
			this->m_components.push_back(component);
			component->OnInit();
			return true;
		}
	}
	default:
		break;
	}


	//Mixed Types
	if ((cType & (Component::Type::Panel | Component::Type::InputBox)) != Component::Type::NONE) {
		auto lastComponent = (this->m_components.empty()) ? std::shared_ptr<Component>() : *(std::prev(this->m_components.end()));
		if (lastComponent == nullptr) {
			component->SetPosition(1, 1);
			this->m_components.push_back(component);
			component->OnInit();
			return true;
		}
		else {
			component->SetPosition(1, lastComponent->GetPosition().Y + 1);
			this->m_components.push_back(component);
			component->OnInit();
			return true;
		}
	}

	
	return false;
}


void Panel::Update(EventProcessor* ep)
{
	if (this->m_fc.Find("OnUpdate"))
		this->m_fc.CallFunction<void>("OnUpdate");

	//Update Components
	for (int i = 0; i < this->m_components.size(); i++) {
		auto& component = this->m_components[i];
		EventProcessor epInstance = *ep;
		epInstance.m_mousePos.X = ep->GetMousePos().X - this->GetPosition().X;
		epInstance.m_mousePos.Y = ep->GetMousePos().Y - this->GetPosition().Y;
		component->Update(&epInstance);
	}

	// Check if the mouse is hovering over the panel
	if (this->CheckInBoundaries(ep->GetMousePos(), {
			this->GetPosition().X,
			this->GetPosition().Y,
			short(this->GetPosition().X + this->GetSize().X),
			short(this->GetPosition().Y + this->GetSize().Y)
		}))
	{
		COORD mPos = {
			short(ep->GetMousePos().X - this->GetPosition().X),
			short(ep->GetMousePos().Y - this->GetPosition().Y)
		};
		this->m_mousePosition = mPos;

		if (this->m_fc.Find("OnMouseHover"))
		{
			this->m_fc.CallFunction<void, const COORD&>("OnMouseHover", mPos);
		}
		this->isHovering = true;
	}
	else {
		this->isHovering = false;
	}

	// Handle mouse press events
	if (ep->Mouse(EventProcessor::MouseType::Left).bStrokePressed)
	{
		if(this->isMovable)
		{
			// Dragging logic
			if (this->CheckInBoundaries(ep->GetMousePos(), {
				this->GetPosition().X,
				short(this->GetPosition().Y - 1),
				short(this->GetPosition().X + this->GetSize().X),
				short(this->GetPosition().Y - 1)
				})) {
				isDragging = true;
				offset = {
					short(ep->GetMousePos().X - this->GetPosition().X),
					short(ep->GetMousePos().Y - this->GetPosition().Y)
				};
			}
		}

		// Targeting logic
		this->m_targeted = this->IsHovering();

		// Resizing logic
		const short resizeMargin = 0; // Buffer zone for resizing interaction

		if(isResizable)
		{
			// Right edge resizing
			if (this->CheckInBoundaries(ep->GetMousePos(), {
				short(this->GetPosition().X + this->GetSize().X - resizeMargin),
				this->GetPosition().Y,
				short(this->GetPosition().X + this->GetSize().X),
				short(this->GetPosition().Y + this->GetSize().Y - 1)
				})) {
				resizeDragging = true;
				this->isResizingRight = true;
				this->m_resizeOffset = {
					short(ep->GetMousePos().X - this->GetPosition().X),
					this->GetSize().Y
				};
			}
			// Bottom edge resizing
			else if (this->CheckInBoundaries(ep->GetMousePos(), {
				short(this->GetPosition().X + 1),
				short(this->GetPosition().Y + this->GetSize().Y - resizeMargin),
				short(this->GetPosition().X + this->GetSize().X - 1),
				short(this->GetPosition().Y + this->GetSize().Y)
				})) {
				resizeDragging = true;
				this->isResizingBottom = true;
				this->m_resizeOffset = {
					this->GetSize().X,
					short(ep->GetMousePos().Y - this->GetPosition().Y)
				};
			}
			// Left edge resizing
			else if (this->CheckInBoundaries(ep->GetMousePos(), {
				short(this->GetPosition().X - 1),
				short(this->GetPosition().Y),
				short(this->GetPosition().X + resizeMargin - 1),
				short(this->GetPosition().Y + this->GetSize().Y - 1)
				})) {
				this->isResizingLeft = true;
				resizeDragging = true;
				this->m_resizeOffset = {
					short(ep->GetMousePos().X - this->GetPosition().X + 1),
					this->GetSize().Y
				};
			}
			else if (this->GetPosition().X + this->GetSize().X == ep->GetMousePos().X &&
				this->GetPosition().Y + this->GetSize().Y == ep->GetMousePos().Y) {
				this->isResizingRight = this->isResizingBottom = true;
				this->resizeDragging = true;
				this->m_resizeOffset = {
					short(ep->GetMousePos().X - this->GetPosition().X),
					short(ep->GetMousePos().Y - this->GetPosition().Y)
				};
			}
			else if (this->GetPosition().X - 1 == ep->GetMousePos().X &&
				this->GetPosition().Y + this->GetSize().Y == ep->GetMousePos().Y) {
				this->isResizingLeft = this->isResizingBottom = true;
				this->resizeDragging = true;
				this->m_resizeOffset = {
					short(ep->GetMousePos().X - this->GetPosition().X),
					short(ep->GetMousePos().Y - this->GetPosition().Y)
				};
			}
		}
	}

	// Handle dragging
	if (ep->Mouse(EventProcessor::MouseType::Left).bStrokeIsHeld)
	{
		if (isDragging)
		{
			this->SetPosition(
				ep->GetMousePos().X - offset.X,
				ep->GetMousePos().Y - offset.Y
			);
		}
		if (resizeDragging)
		{
			// Minimum size constraints
			short minWidth = 0;
			if (this->titleAlignment == TitleAlignment::Center) {
				minWidth = this->GetTitle().length() * 4; // Minimum width
			}
			else if (this->titleAlignment == TitleAlignment::Left) {
				minWidth = this->GetTitle().length() * 2; // Minimum width
			}
			else {
				minWidth = this->GetTitle().length() * 2; // Minimum width
			}
			
			const short minHeight = 1;                          // Minimum height for the panel

			short newX = this->GetPosition().X;  // Default to current X position
			short newWidth = this->GetSize().X; // Default to current width
			short newHeight = this->GetSize().Y; // Default to current height

			// Right-edge resizing
			if (isResizingRight)
			{
				newWidth = ep->GetMousePos().X - this->GetPosition().X;
				if (newWidth < minWidth)
					newWidth = minWidth;
				//this->isResizingRight = false;
			}

			// Left-edge resizing
			if (isResizingLeft)
			{
				short leftDelta = this->GetPosition().X - ep->GetMousePos().X;
				newWidth = this->GetSize().X + leftDelta;

				if (newWidth >= minWidth)
				{
					// Adjust position only if the width is valid
					newX = ep->GetMousePos().X;
				}
				else
				{
					// Prevent width from going below the minimum
					newWidth = minWidth;
					newX = this->GetPosition().X + this->GetSize().X - minWidth;
				}
			}

			// Bottom-edge resizing
			if (isResizingBottom)
			{
				newHeight = ep->GetMousePos().Y - this->GetPosition().Y;
				if (newHeight < minHeight)
					newHeight = minHeight;
			}

			// Apply new position and size
			this->SetPosition(newX, this->GetPosition().Y);
			this->SetSize(newWidth, newHeight, true);
			if (this->GetFunctionContainer().Find("OnResize"))
				this->GetFunctionContainer().CallFunction<void, Panel&>("OnResize", *this);
		}
	}

	// Handle mouse release
	if (ep->Mouse(EventProcessor::MouseType::Left).bStrokeReleased)
	{
		isDragging = false;
		resizeDragging = false;
		this->isResizingLeft = this->isResizingBottom = this->isResizingRight = false;
	}
}

void Panel::Render(RenderTarget* out)
{
	if (this->m_fc.Find("OnLastRender"))
		this->m_fc.CallFunction<void>("OnLastRender");
	for (int i = 0; i < this->m_components.size(); i++)
	{
		auto& component = this->m_components[i];

		component->Render(this);
	}
	//int a = this->GetTitle().length() * (this->GetTitle().length() / this->GetSize().X);
	SetUpFrame(out, this->GetRect(), this->nBorderColor);
	switch (this->titleAlignment) {
	default:
		break;
	case TitleAlignment::Left:
		out->RenderText(this->GetPosition().X + this->titleAlignmentOffset + (this->GetTitle().length() * 0.25),
			this->GetPosition().Y - 1, this->GetTitle(), 0x0f);
		break;

	case TitleAlignment::Center:
		out->RenderText(this->GetPosition().X + (this->GetSize().X / 2u) - (this->GetTitle().length() / 2),
			this->GetPosition().Y - 1, this->GetTitle(), 0x0f);
		break;

	case TitleAlignment::Right:
		out->RenderText(this->GetPosition().X + this->GetSize().X + this->titleAlignmentOffset - (this->GetTitle().length() * 1.25),
			this->GetPosition().Y - 1, this->GetTitle(), 0x0f);
		break;
	}
	
	RenderTarget::FlushTo(out, this->GetRect());
	if (this->m_fc.Find("OnRender"))
		this->m_fc.CallFunction<void, Panel&>("OnRender", *this);
}

void Panel::OnInit()
{
	if (this->GetFunctionContainer().Find("OnInit"))
		this->GetFunctionContainer().CallFunction<void, Panel&>("OnInit", *this);
}

std::string Panel::GetTitle() const
{
	return this->m_title;
}

COORD Panel::GetMousePosition() const
{
	return this->m_mousePosition;
}
