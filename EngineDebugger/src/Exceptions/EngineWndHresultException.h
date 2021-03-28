#pragma once

#include "EngineException.h"
#include "../Win.h"

namespace EngineExcept
{
	class __declspec(dllexport) EngineWndHresultException : public EngineException
	{
	public:
		EngineWndHresultException(int line, const char* file, HRESULT hr);
	public:
		const char* what() const override;
		const char* GetExceptionType() const override;
		const char* GetErrorDescription(HRESULT hresult) const;
	private:
		HRESULT hr;
	};

#define WND_HR_EXCEPT(hr) EngineExcept::EngineWndHresultException(__LINE__, __FILE__, hr);
#define WND_LAST_EXCEPT() EngineExcept::EngineWndHresultException(__LINE__, __FILE__, GetLastError());
}