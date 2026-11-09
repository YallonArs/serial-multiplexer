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

protected:
	string name;

private:
	static constexpr const char* LOG_FILE_PATH = "journal.log";
	static std::ofstream logFile;

	string levelToString(const Level level) const noexcept;
	static string formatTimestamp();
	string formatMessage(const Level level, const string &message) const;

public:
	Logger() = default;
	Logger(const string &name);

	void set_name(const string &name);

	void log(const Level level, const string &message) const;
	
	void debug(const string &message) const;
	void info(const string &message) const;
	void warning(const string &message) const;
	void error(const string &message) const;
};
