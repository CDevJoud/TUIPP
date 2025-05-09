#pragma once
#include <tuple>
#include <memory>
#include <any>
#include <functional>
#include <string>
#include <map>
#include <Windows.h>
class FunctionContainer
{
public:
	FunctionContainer(std::initializer_list<std::pair<std::string, std::any>> initlist);

	template <typename Func>
	void AddFunction(const std::string& name, Func func)
	{
		this->m_functions[name] = func;
	}

	template <typename Ret, typename... Args>
	Ret CallFunction(const std::string& name, Args&&... args)
	{
		if (this->m_functions.find(name) != this->m_functions.end())
		{
			auto& func = this->m_functions[name];
			auto castedFunc = std::any_cast<std::function<Ret(Args...)>>(func);
			return castedFunc(std::forward<Args>(args)...);
		}
		else
		{
			OutputDebugStringA(std::string("Function \"" + name + "\"" " not found!").c_str());
		}
	}

	bool Find(const std::string& name);

	template <typename Ret, typename... Args>
	std::function<Ret(Args...)> operator[](const std::string& name)
	{
		if (this->m_functions.find(name) != this->m_functions.end())
		{
			return std::any_cast<std::function<Ret(Args...)>>(this->m_functions[name]);
		}
		else
			OutputDebugStringA(std::string("Function \"" + name + "\"" " not found!").c_str());
	}
	template <typename Ret, typename... Args>
	std::function<Ret(Args...)> GetFunction(const std::string& name)
	{
		if (this->m_functions.find(name) != this->m_functions.end())
		{
			try
			{
				return std::any_cast<std::function<Ret(Args...)>>(this->m_functions[name]);
			}
			catch (const std::bad_any_cast& e)
			{
				OutputDebugStringA(std::string("Failed to cast function \"" + name + "\": " + e.what()).c_str());
			}
		}
		else
		{
			OutputDebugStringA(std::string("Function \"" + name + "\" not found!").c_str());
		}
		return nullptr; // Return a default-constructed callable if not found or on error
	}
private:
	std::map<std::string, std::any> m_functions;
};