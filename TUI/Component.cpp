#include "Component.hpp"

Component::Component(Type cType, RenderElement* re, const std::string& ID, FunctionContainer fc) :
	m_type(cType),
	m_re(re),
	m_id(ID),
	m_targeted(false),
	m_fc(fc)
{

}

void Component::SetSize(const uint16_t width, const uint16_t height, bool buf)
{
	this->m_nWidth = width;
	this->m_nHeight = height;
	if (buf)
	{
		int size = this->m_nWidth * this->m_nHeight;
		if (this->m_re->buffer.size() != size)
		{
			this->m_re->viewRect.Right = this->m_nWidth;
			this->m_re->viewRect.Bottom = this->m_nHeight;
			this->m_re->buffer.shrink_to_fit();
			this->m_re->buffer.resize(size);
		}
	}
}

void Component::SetPosition(const uint16_t x, const uint16_t y)
{
	this->m_nPositionX = x;
	this->m_nPositionY = y;
}

COORD Component::GetPosition() const
{
	return {short(this->m_nPositionX), short(this->m_nPositionY)};
}

COORD Component::GetSize() const
{
	return { short(this->m_nWidth), short(this->m_nHeight) };
}

std::string Component::GetID() const
{
	return this->m_id;
}

COORD Component::GetDefaultPosition() const
{
	return this->m_defaultPosition;
}

Component::Type Component::GetType() const
{
    return this->m_type;
}

FunctionContainer& Component::GetFunctionContainer()
{
	return this->m_fc;
}

RenderElement* Component::GetRenderElement() const
{
	return this->m_re;
}
