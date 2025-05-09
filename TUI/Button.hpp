#pragma once
#include "Component.hpp"
#include "FunctionContainer.hpp"
#include <stdexcept>

class Button : public RenderTarget, public Component
{
public:
	Button() = default;
	Button(const std::string& title, uint16_t width = Button::Default, uint16_t height = Button::Default, FunctionContainer fc = {});
	Button(const std::string& title, uint16_t width = Button::Default, uint16_t height = Button::Default, bool withFrame = false, FunctionContainer fc = {});
	template <typename TemplateFunctionContainer>
	Button(const std::string& title, uint16_t width, uint16_t height, TemplateFunctionContainer&& fc) :
	m_nWidth(width),
	m_nHeight(height),
	m_title(title),
	m_fc(std::forward<TemplateFunctionContainer>(fc)),
	Component(Component::Type::Button, &this->re, title)
	{
		if (typeid(std::decay_t<TemplateFunctionContainer>) != typeid(FunctionContainer))
		{
			throw std::invalid_argument("Invalid type provided. Expected FunctionContainer.");
		}
		this->Create(title, width, height);
	}
	void Create(const std::string& title, uint16_t width = 0xffff, uint16_t height = 0xff);
	
	static std::shared_ptr<Button> CreateInstance(const std::string& title, uint16_t width = 0xffff, uint16_t height = 0xffff, FunctionContainer fc = {});
	static std::shared_ptr<Button> CreateInstance(const std::string& title, uint16_t width = 0xffff, uint16_t height = 0xffff, bool withFrame = false, FunctionContainer fc = {});
	template<typename T>
	static std::shared_ptr<Button>CreateInstance(const std::string& title, uint16_t width, uint16_t height, T&& fc)
	{
		if (typeid(std::decay_t<T>) != typeid(FunctionContainer))
		{
			throw std::invalid_argument("Invalid type provided. Expected FunctionContainer.");
		}
		auto component = std::make_shared<Button>(title, width, height, fc);
		return component;
	}
	void Update(EventProcessor* ep) override;
	void Render(RenderTarget* out) override;
	void OnInit() override;

	void WithFrame(bool value);
	std::string GetTitle() const;
	void SetTextColor(const uint8_t color);
	static inline constexpr int Default = 0xffff;
private:
	void SetUpFrame(RenderTarget* rt, SMALL_RECT rect, uint8_t color);
	std::string m_title;
	uint8_t : 4ui8, m_color = 0x00ui8, m_frontColor = 0x0Fui8;
	uint8_t m_hoveringColor, m_btnColor;
	bool m_isHovering, m_withFrame;
};