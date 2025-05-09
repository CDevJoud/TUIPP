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
	Component(Component::Type::Panel, &this->re, "NONE!")
{

}

Panel::Panel(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc) :
	m_title(title),
	Component(Component::Type::Panel, &this->re, title, fc)
{
	this->Create(title, width, height);
}

std::shared_ptr<Panel> Panel::CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc)
{
	auto component = std::make_shared<Panel>(title, width, height, fc);
	component->SetPosition(1, 1);
	return component;
}

void Panel::Create(const std::string& title, uint16_t width, uint16_t height)
{
	this->m_nWidth = width;
	this->m_nHeight = height;
	this->m_title = title;
	this->re.buffer.resize(width * height);
	this->re.viewRect = { 0, 0, (short)width, (short)height };
}

bool Panel::InsertComponent(const std::shared_ptr<Component>& component)
{
	auto cType = component->GetType();
	switch (cType)
	{
	case Component::Type::Panel:
		m_components.push_back(component);
		break;
	case Component::Type::Button:
	{
		auto lastComponent = (this->m_components.empty()) ? std::shared_ptr<Component>() : *(std::prev(this->m_components.end()));
		if(lastComponent == nullptr)
		{
			component->SetPosition(0, 0);
			this->m_components.push_back(component);
			component->OnInit();
		}
		else
		{
			component->SetPosition(0, lastComponent->GetPosition().Y + 1);
			this->m_components.push_back(component);
			component->OnInit();
		}
	}
		break;
	default:
		break;
	}
	
	return true;
}


void Panel::Update(EventProcessor* ep)
{
	

	if (this->m_fc.Find("OnUpdate"))
		this->m_fc.CallFunction<void>("OnUpdate");

	this->m_testMousePos = ep->GetMousePos();
	bool bIsHovered = false;
	
	// Check if the mouse is hovering over the panel
	if (this->CheckInBoundaries(ep->GetMousePos(), {
			this->GetPosition().X,
			this->GetPosition().Y,
			short(this->GetPosition().X + this->GetSize().X),
			short(this->GetPosition().Y + this->GetSize().Y)
		}))
	{
		for (int i = 0; i < this->m_components.size(); i++)
		{
			auto& component = this->m_components[i];
			EventProcessor epInstance = *ep;
			epInstance.m_mousePos.X = ep->GetMousePos().X - this->GetPosition().X;
			epInstance.m_mousePos.Y = ep->GetMousePos().Y - this->GetPosition().Y;
			component->Update(&epInstance);
		}
		COORD mPos = {
			short(ep->GetMousePos().X - this->GetPosition().X),
			short(ep->GetMousePos().Y - this->GetPosition().Y)
		};
		this->m_mousePosition = mPos;

		if (this->m_fc.Find("OnMouseHover"))
		{
			this->m_fc.CallFunction<void, const COORD&>("OnMouseHover", mPos);
		}
		bIsHovered = true;
	}

	// Handle mouse press events
	if (ep->Mouse(EventProcessor::MouseType::Left).bStrokePressed)
	{
		// Dragging logic
		if (this->CheckInBoundaries(ep->GetMousePos(), {
			this->GetPosition().X,
			short(this->GetPosition().Y - 1),
			short(this->GetPosition().X + this->GetSize().X),
			short(this->GetPosition().Y - 1)
			}))
		{
			isDragging = true;
			offset = {
				short(ep->GetMousePos().X - this->GetPosition().X),
				short(ep->GetMousePos().Y - this->GetPosition().Y)
			};
		}

		// Targeting logic
		this->m_targeted = bIsHovered;

		// Resizing logic
		const short resizeMargin = 0; // Buffer zone for resizing interaction

		// Right edge resizing
		if (this->CheckInBoundaries(ep->GetMousePos(), {
			short(this->GetPosition().X + this->GetSize().X - resizeMargin),
			this->GetPosition().Y,
			short(this->GetPosition().X + this->GetSize().X),
			short(this->GetPosition().Y + this->GetSize().Y - 1)
			}))
		{
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
			}))
		{
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
			}))
		{
			this->isResizingLeft = true;
			resizeDragging = true;
			this->m_resizeOffset = {
				short(ep->GetMousePos().X - this->GetPosition().X + 1),
				this->GetSize().Y
			};
		}
		else if (this->GetPosition().X + this->GetSize().X == ep->GetMousePos().X &&
			this->GetPosition().Y + this->GetSize().Y == ep->GetMousePos().Y)
		{
			this->isResizingRight = this->isResizingBottom = true;
			this->resizeDragging = true;
			this->m_resizeOffset = {
				short(ep->GetMousePos().X - this->GetPosition().X),
				short(ep->GetMousePos().Y - this->GetPosition().Y)
			};
		}
		else if (this->GetPosition().X - 1 == ep->GetMousePos().X &&
			this->GetPosition().Y + this->GetSize().Y == ep->GetMousePos().Y)
		{
			this->isResizingLeft = this->isResizingBottom = true;
			this->resizeDragging = true;
			this->m_resizeOffset = {
				short(ep->GetMousePos().X - this->GetPosition().X),
				short(ep->GetMousePos().Y - this->GetPosition().Y)
			};
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
			const short minWidth = this->GetTitle().length() * 4; // Minimum width
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
				this->GetFunctionContainer().CallFunction<void>("OnResize");
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
	SetUpFrame(out, this->GetRect(), 0x0f);
	out->RenderText(this->GetPosition().X + (this->GetSize().X / 2u) - (this->GetTitle().length() / 2),
		this->GetPosition().Y - 1, this->GetTitle(), 0x0f);
	RenderTarget::FlushTo(out, this->GetRect());
	if (this->m_fc.Find("OnRender"))
		this->m_fc.CallFunction<void>("OnRender");
}

void Panel::OnInit()
{
	if (this->GetFunctionContainer().Find("OnInit"))
		this->GetFunctionContainer().CallFunction<void>("OnInit");
}

std::string Panel::GetTitle() const
{
	return this->m_title;
}

COORD Panel::GetMousePosition() const
{
	return this->m_mousePosition;
}
