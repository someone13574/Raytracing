#include "EngineLogger.h"

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace EngineDebug
{
	void EngineLogger::ClearLog()
	{
		std::ofstream("../log.txt", std::ios::out | std::ios::trunc) << "";
		DEBUGLOG("Cleared Log!");
	}

	void EngineLogger::Log(std::string message, std::string file, int line, int severity)
	{
		const char* severities[3] = { "[--Log--]", "[Warning]", "[-Error-]" };

		time_t now = time(0);
		tm localTime;
		localtime_s(&localTime, &now);

		const size_t lastSlashIndex = file.find_last_of("\\");
		if (std::string::npos != lastSlashIndex)
			file.erase(0, lastSlashIndex + 1);

		std::ostringstream string;
		string << severities[severity] << "[" << localTime.tm_hour << ":" << localTime.tm_min << ":" << localTime.tm_sec << "][" << file << ", " << std::setw(3) << std::setfill('0') << line << "]   " << message << std::endl;
		std::ofstream("../log.txt", std::ios::app) << string.str();
	}
	void EngineLogger::Log(int message, std::string file, int line, int severity)
	{
		const char* severities[3] = { "[--Log--]", "[Warning]", "[-Error-]" };

		time_t now = time(0);
		tm localTime;
		localtime_s(&localTime, &now);

		const size_t lastSlashIndex = file.find_last_of("\\");
		if (std::string::npos != lastSlashIndex)
			file.erase(0, lastSlashIndex + 1);

		std::ostringstream string;
		string << severities[severity] << "[" << localTime.tm_hour << ":" << localTime.tm_min << ":" << localTime.tm_sec << "][" << file << ", " << std::setw(3) << std::setfill('0') << line << "]   " << message << std::endl;
		std::ofstream("../log.txt", std::ios::app) << string.str();
	}
	void EngineLogger::Log(float message, std::string file, int line, int severity)
	{
		const char* severities[3] = { "[--Log--]", "[Warning]", "[-Error-]" };

		time_t now = time(0);
		tm localTime;
		localtime_s(&localTime, &now);

		const size_t lastSlashIndex = file.find_last_of("\\");
		if (std::string::npos != lastSlashIndex)
			file.erase(0, lastSlashIndex + 1);

		std::ostringstream string;
		string << severities[severity] << "[" << localTime.tm_hour << ":" << localTime.tm_min << ":" << localTime.tm_sec << "][" << file << ", " << std::setw(3) << std::setfill('0') << line << "]   " << message << std::endl;
		std::ofstream("../log.txt", std::ios::app) << string.str();
	}
	void EngineLogger::Log(unsigned int message, std::string file, int line, int severity)
	{
		const char* severities[3] = { "[--Log--]", "[Warning]", "[-Error-]" };

		time_t now = time(0);
		tm localTime;
		localtime_s(&localTime, &now);

		const size_t lastSlashIndex = file.find_last_of("\\");
		if (std::string::npos != lastSlashIndex)
			file.erase(0, lastSlashIndex + 1);

		std::ostringstream string;
		string << severities[severity] << "[" << localTime.tm_hour << ":" << localTime.tm_min << ":" << localTime.tm_sec << "][" << file << ", " << std::setw(3) << std::setfill('0') << line << "]   " << message << std::endl;
		std::ofstream("../log.txt", std::ios::app) << string.str();
	}
}