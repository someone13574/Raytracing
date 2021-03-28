#pragma once

#include "EngineException.h"

#include <vector>

namespace EngineExcept
{
	class __declspec(dllexport) EngineGFXInfoException : public EngineException
	{
	public:
		EngineGFXInfoException(int line, const char* file, std::vector<std::string> infoMessages = {});
	public:
		const char* what() const override;
		const char* GetExceptionType() const override;
	private:
#pragma warning(push)
#pragma warning(disable:4251)
		std::string info;
#pragma warning(pop)
	};

#ifndef NDEBUG
#define GFX_THROW_INFO_ONLY(call) errorCatcher.Set(); (call); {auto v = errorCatcher.GetMessages(); if (!v.empty()) {throw EngineExcept::EngineGFXInfoException(__LINE__, __FILE__, v);}}

#define GFX_INFO_START() errorCatcher.Set();
#define GFX_INFO_END() {auto v = errorCatcher.GetMessages(); if (!v.empty()) { throw EngineExcept::EngineGFXInfoException(__LINE__, __FILE__, v);}}
#else
#define GFX_THROW_INFO_ONLY(call) (call)
#endif
}