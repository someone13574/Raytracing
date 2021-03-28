#include "EngineGFXHresultException.h"
#include "../EngineLogger.h"

#include <sstream>

namespace EngineExcept
{
	EngineGFXHresultException::EngineGFXHresultException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMessages) : EngineException(line, file), hr(hr)
	{
		EngineDebug::EngineLogger::Log(GetErrorDescription(hr), file, line, 2);

		for (const auto& m : infoMessages)
		{
			info += m;
			info.push_back('\n');
		}
		if (!info.empty())
		{
			info.pop_back();
		}
	}

	const char* EngineGFXHresultException::what() const
	{
		std::ostringstream string;
		string << GetExceptionType() << std::endl << std::endl
			<< "File: " << GetFile() << std::endl
			<< "Line: " << GetLine() << std::endl << std::endl
			<< "Description: " << GetErrorDescription(hr);

		if (!info.empty())
			string << std::endl << "Error Info: " << std::endl << info;

		return string.str().c_str();
	}

	const char* EngineGFXHresultException::GetExceptionType() const
	{
		return "Engine Graphics HRESULT Exception";
	}

	const char* EngineGFXHresultException::GetErrorDescription(HRESULT hresult) const
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