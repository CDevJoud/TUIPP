#pragma once
#include "Window.hpp"
#include "EventProcessor.hpp"
#include "Panel.hpp"
#include "Button.hpp"
#include <memory>

class Console : public Window, public RenderTarget, public EventProcessor
{
public:
	Console(const std::string& title, uint16_t width, uint16_t height, uint16_t pixelWidth, uint16_t pixelHeight);
	~Console();
	HANDLE GetOutputHandle() const;
	HANDLE GetInputHandle() const;

	COORD GetConsoleSize() const;
	SMALL_RECT GetViewRect() const;

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

	void SetConsoleSize(uint16_t width, uint16_t height, bool updateBuffer = false);

	void Display();
private:
	uint32_t m_targetedComponent;
	std::vector<std::shared_ptr<Component>> m_components;
	HANDLE m_hOutConsole, m_hInConsole;
	uint16_t m_nPixelWidth, m_nPixelHeight, m_nWidth, m_nHeight;
	SMALL_RECT m_ViewRect;
};