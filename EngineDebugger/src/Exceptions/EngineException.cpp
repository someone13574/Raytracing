#include "EngineException.h"

#include <sstream>
#include <fstream>

namespace EngineExcept
{
	EngineException::EngineException(int line, const char* file) : line(line), file(file) { }

	const char* EngineException::what() const
	{
		std::ostringstream string;
		string << "Engine Exception" << std::endl << "File: " << file << std::endl << "Line: " << line;
		return string.str().c_str();
	}

	const char* EngineException::GetExceptionType() const
	{
		return "Engine Exception";
	}

	const char* EngineException::GetFile() const
	{
		return file;
	}

	int EngineException::GetLine() const
	{
		return line;
	}
}