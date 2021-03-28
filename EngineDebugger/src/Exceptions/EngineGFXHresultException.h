#pragma once

#include "EngineException.h"
#include "../Win.h"

#include <vector>

namespace EngineExcept
{
	class __declspec(dllexport) EngineGFXHresultException : public EngineException
	{
	public:
		EngineGFXHresultException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMessages = {});
	public:
		const char* what() const override;
		const char* GetExceptionType() const override;

		const char* GetErrorDescription(HRESULT hresult) const;
	private:
		HRESULT hr;
#pragma warning(push)
#pragma warning(disable:4251)
		std::string info;
#pragma warning(pop)
	};

#define GFX_EXCEPT_NOINFO(hr) EngineExcept::EngineGFXHresultException(__LINE__, __FILE__, hr);
#define GFX_THROW_NOINFO(hrcall) if (FAILED(hr = (hrcall))) throw EngineExcept::EngineGFXHresultException(__LINE__, __FILE__, hr);

#ifndef NDEBUG
#define GFX_EXCEPT(hr) EngineExcept::EngineGFXHresultException(__LINE__, __FILE__, (hr), errorCatcher.GetMessages())
#define GFX_THROW_INFO(hrcall) errorCatcher.Set(); if (FAILED(hr = (hrcall))) throw GFX_EXCEPT(hr);
#else
#define GFX_EXCEPT(hr) EngineExcept::EngineGFXHresultException(__LINE__, __FILE__, hr);
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall);
#endif
}