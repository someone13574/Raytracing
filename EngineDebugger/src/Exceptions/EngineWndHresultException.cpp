#include "EngineWndHresultException.h"
#include "../EngineLogger.h"

#include <sstream>

namespace EngineExcept
{
	EngineWndHresultException::EngineWndHresultException(int line, const char* file, HRESULT hr) : EngineException(line, file), hr(hr)
	{
		EngineDebug::EngineLogger::Log(GetErrorDescription(hr), file, line, 2);
	}

	const char* EngineWndHresultException::what() const
	{
		std::ostringstream string;
		string << GetExceptionType() << std::endl << std::endl
			<< "File: " << GetFile() << std::endl
			<< "Line: " << GetLine() << std::endl << std::endl
			<< "Description: " << GetErrorDescription(hr);
		return string.str().c_str();
	}

	const char* EngineWndHresultException::GetExceptionType() const
	{
		return "Engine Window HRESULT Exception";
	}

	const char* EngineWndHresultException::GetErrorDescription(HRESULT hresult) const
	{
		char* pMsgBuffer = nullptr;
		DWORD msgLength = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&pMsgBuffer), 0, nullptr);
		if (msgLength == 0)
			return "Error Code Not Found...";
		const char* errMsg = pMsgBuffer;
		LocalFree(pMsgBuffer);
		return errMsg;
	}
}