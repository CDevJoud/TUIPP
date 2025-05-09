#pragma once
#include "Component.hpp"
#include "FunctionContainer.hpp"
#include <stdexcept>

/// 
/// supported FunctionContainer functions:
/// OnUpdate() // calls for updating the panel
/// OnLastUpdate() // calls after everything updates
/// OnRender() // calls for render the panel and its content
/// OnLastRender() // calls after everything renders
/// OnMouseClicked(EventProcessor::MouseType mType, COORD mPos) // calls when mouse clicked on the panel
/// OnMouseHover(const COORD& mPos) // calls when mouse hover over the panel
/// OnDestroy() // calls when the components destroys
/// 

class Panel : public virtual RenderTarget, public Component
{
public:
	Panel();
	Panel(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc = {});
	template <typename TemplateFunctionContainer>
	Panel(const std::string& title, uint16_t width, uint16_t height, TemplateFunctionContainer&& fc) :
	m_nWidth(width),
	m_nHeight(height),
	m_title(title),
	m_nPositionX(0),
	m_nPositionY(0),
	Component(Component::Type::Panel, &this->re, title, std::forward<TemplateFunctionContainer>(fc))
	{
		if (typeid(std::decay_t<TemplateFunctionContainer>) != typeid(FunctionContainer))
		{
			throw std::invalid_argument("Invalid type provided. Expected FunctionContainer.");
		}
		this->Create(title, width, height);
	}

	static std::shared_ptr<Panel> CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc = {});
	template <typename TemplateFunctionContainer>
	static std::shared_ptr<Panel> CreateInstance(const std::string& title, uint16_t width, uint16_t height, TemplateFunctionContainer&& fc)
	{
		if (typeid(std::decay_t<TemplateFunctionContainer>) != typeid(FunctionContainer))
		{
			throw std::invalid_argument("Invalid type provided. Expected FunctionContainer.");
		}
		auto component = std::make_shared<Panel>(title, width, height, fc);
		component->SetPosition(1, 1);
		return component;
	}
	void Create(const std::string& title, uint16_t width, uint16_t height);
	bool InsertComponent(const std::shared_ptr<Component>& component);
	template<typename CompType>
	std::shared_ptr<CompType> GetComponent(const std::string& title)
	{
		for (auto& i : this->m_components)
		{
			if (i->GetID() == title)
			{
				return std::dynamic_pointer_cast<CompType>(i);
			}
		}
		return nullptr;
	}

	void Update(EventProcessor* ep) override;
	void Render(RenderTarget* out) override;
	void OnInit() override;

	std::string GetTitle() const;
	COORD GetMousePosition() const;

	//Get Pos.xy and size.xy
	SMALL_RECT GetRect() const;
protected:
	void SetUpFrame(RenderTarget* out, SMALL_RECT rect, uint8_t color);
private:
	std::vector<std::shared_ptr<Component>> m_components;
	// Initialize resizing direction flags
	bool isResizingRight = false;
	bool isResizingLeft = false;
	bool isResizingBottom = false;
	COORD m_mousePosition, m_testMousePos;
	bool isDragging = false, resizeDragging = false;
	COORD offset{}, m_resizeOffset;
	std::string m_title;
	//uint16_t m_nWidth, m_nHeight, m_nPositionX, m_nPositionY;
};

