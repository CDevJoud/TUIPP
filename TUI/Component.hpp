#pragma once
#include "RenderTarget.hpp"
#include "EventProcessor.hpp"
#include "FunctionContainer.hpp"

class Component
{
public:
	enum class Type
	{
		Panel = 0,
		Button
	};
	//Function container will be added!
	Component(Type cType, RenderElement* re, const std::string& ID, FunctionContainer fc = {});

	virtual void Update(EventProcessor* ep) = 0;
	virtual void Render(RenderTarget* out) = 0;
	virtual void OnInit() = 0;

	virtual void SetSize(const uint16_t width, const uint16_t height, bool resizeBuffer = false);
	virtual void SetPosition(const uint16_t x, const uint16_t y);

	COORD GetPosition() const;
	COORD GetSize() const;
	std::string GetID() const;

	COORD GetDefaultPosition() const;
	Type GetType() const;

	FunctionContainer& GetFunctionContainer();
protected:
	FunctionContainer m_fc;
	bool m_targeted;
	RenderElement* GetRenderElement() const;
	uint16_t m_nWidth, m_nHeight, m_nPositionX, m_nPositionY;
private:
	std::string m_id;
	COORD m_defaultPosition;
	Type m_type;
	RenderElement* m_re;
	friend class Console;
};

