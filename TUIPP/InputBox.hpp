#pragma once
#include "Panel.hpp"

class InputBox : public Panel {
public:
	enum class Type {
		NONE = 0,
		String = 0b0001,
		Number = 0b0010,
		Decimal = 0b0100,
		Password = -1
	};
	InputBox();
	InputBox(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc = {});
	InputBox(const std::string& title, const std::string& inputValue, uint16_t width, uint16_t height, FunctionContainer fc = {});

	void Render(RenderTarget* out) override;
	void Update(EventProcessor* ep) override;
	static std::shared_ptr<InputBox> CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc = {});
	static std::shared_ptr<InputBox> CreateInstance(const std::string& title, const std::string& inputValue, uint16_t width, uint16_t height, FunctionContainer fc = {});

	InputBox& SetInputType(Type type);
	union {
		char* str;
		int64_t num;
		double decimal;
	}value;

	Panel& GetPanel();
private:
	std::string strInput;
	bool isInputting;
	Type type;
};

inline InputBox::Type operator|(InputBox::Type a, InputBox::Type b) {
	return static_cast<InputBox::Type>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline InputBox::Type operator&(InputBox::Type a, InputBox::Type b) {
	return static_cast<InputBox::Type>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

