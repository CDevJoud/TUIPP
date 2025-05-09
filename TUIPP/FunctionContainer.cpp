#include "FunctionContainer.hpp"

FunctionContainer::FunctionContainer(std::initializer_list<std::pair<std::string, std::any>> initList)
{
	for (const auto& [name, func] : initList) {
		this->m_functions[name] = func;
	}
}

bool FunctionContainer::Find(const std::string& name)
{
	return (this->m_functions.count(name));
}
