#pragma once

#include <exception>
#include <string>

#pragma warning( push )
#pragma warning( disable : 4275)
namespace EngineExcept
{
	class __declspec(dllexport) EngineException : public std::exception
	{
	public:
		EngineException(int line, const char* file);
	public:
		const char* what() const override;
		virtual const char* GetExceptionType() const;
		const char* GetFile() const;
		int GetLine() const;
	private:
		int line;
		const char* file;
	};
}
#pragma warning( pop ) 