#pragma once

#include <fstream>

namespace EngineDebug
{
	class __declspec(dllexport) EngineLogger
	{
	public:
		static void ClearLog();

		static void Log(std::string message, std::string file, int line, int severity);
		static void Log(int message, std::string file, int line, int severity);
		static void Log(float message, std::string file, int line, int severity);
		static void Log(unsigned int message, std::string file, int line, int severity);
	};

#define DEBUGLOG(message) EngineDebug::EngineLogger::Log(message, __FILE__, __LINE__, 0);
#define DEBUGWARN(message) EngineDebug::EngineLogger::Log(message, __FILE__, __LINE__, 1);
#define DEBUGERROR(message) EngineDebug::EngineLogger::Log(message, __FILE__, __LINE__, 2);
}