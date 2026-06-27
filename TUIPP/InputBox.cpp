#include "InputBox.hpp"

InputBox::InputBox() : isInputting(false), type(Type::String){
	Component::SetComponentType(Component::Type::Panel | Component::Type::InputBox);
	memset(&value, 0, 8);
}

InputBox::InputBox(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc) :
	Panel(title, width, height, fc),
	isInputting(false), type(Type::String) {
	Component::SetComponentType(Component::Type::Panel | Component::Type::InputBox);
	memset(&value, 0, 8);
}

InputBox::InputBox(const std::string& title, const std::string& inputValue, uint16_t width, uint16_t height, FunctionContainer fc) :
	Panel(title, width, height, fc),
	strInput(inputValue),
	isInputting(false), type(Type::String) {
	Component::SetComponentType(Component::Type::Panel | Component::Type::InputBox);
	memset(&value, 0, 8);
}

void InputBox::Render(RenderTarget* out) {
	Panel::Render(out);
	RenderTarget::ClearScreen();
	if (InputBox::type == InputBox::Type::Password) {
		RenderTarget::RenderText(0, 0, std::string(strInput.length(), '*'), 0x0f);
	}
	else {
		RenderTarget::RenderText(0, 0, strInput, 0x0f);
	}
}

void InputBox::Update(EventProcessor* ep) {
	Panel::Update(ep);
	if (Panel::IsHovering()) {
		if (ep->Mouse(EventProcessor::MouseType::Left).bStrokeReleased) {
			Panel::SetBorderColor(0x0f);
			this->isInputting = true;
		}
	}
	else {
		if (ep->Mouse(EventProcessor::MouseType::Left).bStrokeIsHeld) {
			Panel::SetBorderColor(0x08);
			this->isInputting = false;
		}
	}

	if(this->isInputting) {
		char c = ep->GetText();
		if (c != 0) {
			if (c == VK_BACK) {
				if (!strInput.empty())
					strInput.pop_back();
			}
			else {
				if (InputBox::type == InputBox::Type::String || InputBox::type == InputBox::Type::Password) {
					strInput += c;
				}
				else if (InputBox::type == InputBox::Type::Number) {
					if (std::isdigit(c)) {
						strInput += c;
					}
				}
				else if ((InputBox::type & (InputBox::Type::Decimal | InputBox::Type::Number)) != InputBox::Type::NONE) {
					if (std::isdigit(c) || c == '.') {
						if (c == '.') {
							if (strInput.find('.') == std::string::npos) {
								strInput += c; // only one decimal point allowed
							}
						}
						else {
							strInput += c; // always allow digits
						}
					}
				}
			}
		}
	}

	if ((InputBox::type & (InputBox::Type::Decimal | InputBox::Type::Number)) != InputBox::Type::NONE) {
		InputBox::value.num = InputBox::value.decimal = atof(this->strInput.data());
	}

	switch (InputBox::type) {
	case Type::NONE:
		break;

	case Type::Decimal:
		InputBox::value.decimal = atof(this->strInput.data());
		break;

	case Type::Number:
		InputBox::value.num = _atoi64(this->strInput.data());
		break;

	case Type::Password:
	case Type::String:
		InputBox::value.str = this->strInput.data();
		break;
	}
}

std::shared_ptr<InputBox> InputBox::CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc) {
	auto component = std::make_shared<InputBox>(title, width, height, fc);
	component->SetPosition(1, 1);
	component->SetResizability(false);
	component->SetMovability(false);
	return component;
}

std::shared_ptr<InputBox> InputBox::CreateInstance(const std::string& title, const std::string& inputValue, uint16_t width, uint16_t height, FunctionContainer fc) {
	auto component = std::make_shared<InputBox>(title, inputValue , width, height, fc);
	component->SetPosition(1, 1);
	component->SetResizability(false);
	component->SetMovability(false);
	return component;
}

InputBox& InputBox::SetInputType(Type type) {
	this->type = type;

	return *this;
}

Panel& InputBox::GetPanel()
{
	// TODO: insert return statement here
	return *this;
}
