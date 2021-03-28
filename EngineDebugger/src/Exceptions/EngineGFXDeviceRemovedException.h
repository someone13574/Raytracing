#pragma once

#include "EngineException.h"
#include "../Win.h"

#include <vector>

namespace EngineExcept
{
	class __declspec(dllexport) EngineGFXDeviceRemovedException : public EngineException
	{
	public:
		EngineGFXDeviceRemovedException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMessages = {});
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

#ifndef NDEBUG
#define GFX_DEVICE_REMOVED_EXCEPT(hr) EngineExcept::EngineGFXDeviceRemovedException(__LINE__, __FILE__, hr, errorCatcher.GetMessages())
#else
#define GFX_DEVICE_REMOVED_EXCEPT(hr) EngineExcept::EngineGFXDeviceRemovedException(__LINE__, __FILE__, hr)
#endif
}