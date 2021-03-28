#include "EngineGFXInfoException.h"

#include <sstream>

namespace EngineExcept
{
	EngineGFXInfoException::EngineGFXInfoException(int line, const char* file, std::vector<std::string> infoMessages) : EngineException(line, file)
	{
		for (const auto& m : infoMessages)
		{
			info += m;
			info.push_back('\n');
		}
		if (!info.empty())
			info.pop_back();
	}

	const char* EngineGFXInfoException::what() const
	{
		std::ostringstream string;
		string << GetExceptionType() << std::endl << std::endl
			<< "File: " << GetFile() << std::endl
			<< "Line: " << GetLine() << std::endl << std::endl
			<< "Error Info: " << info;

		return string.str().c_str();
	}

	const char* EngineGFXInfoException::GetExceptionType() const
	{
		return "Graphics Info Exception";
	}
}