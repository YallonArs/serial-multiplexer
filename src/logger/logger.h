#pragma once

#include <iostream>
#include <string>
#include <fstream>

#include "utils.h"

class Logger {
public:
	enum class Level {
		Debug,
		Info,
		Warning,
		Error
	};

private:
	static constexpr const char* LOG_FILE_PATH = "journal.log";
	static std::ofstream logFile;
	string name;

	string levelToString(const Level level) const;
	static string formatTimestamp();
	string formatMessage(const Level level, const std::string &message) const;

public:
	Logger(const string name);

	void log(const Level level, const std::string &message) const;
	
	void debug(const std::string &message) const;
	void info(const std::string &message) const;
	void warning(const std::string &message) const;
	void error(const std::string &message) const;
};
