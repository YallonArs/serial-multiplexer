#include <chrono>
#include <format>
#include <sstream>
#include "Logger.h"

std::ofstream Logger::logFile(Logger::LOG_FILE_PATH, std::ios::app);

Logger::Logger(const string name) : name(name) {}

string Logger::levelToString(const Level level) const {
	switch (level) {
		case Level::Debug:
			return "DEBUG";
		case Level::Info:
			return "INFO";
		case Level::Warning:
			return "WARNING";
		case Level::Error:
			return "ERROR";
		default:
			return "UNKNOWN";
	}
}

string Logger::formatTimestamp() {
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::tm now_tm;
	localtime_r(&now_time_t, &now_tm);

	char buf[20];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &now_tm);
	std::string timestamp = buf;
	timestamp += std::format(".{:03d}", now_ms.count());

	return timestamp;
}

string Logger::formatMessage(const Level level, const std::string &message) const {
	return std::format("[{}] ({}) {}: {}", formatTimestamp(), name, levelToString(level), message);
}

void Logger::log(const Level level, const std::string &message) const {
	std::string formattedMessage = formatMessage(level, message);
	
	std::cout << formattedMessage << std::endl;
	logFile << formattedMessage << std::endl;
}

void Logger::debug(const std::string &message) const {
	log(Level::Debug, message);
}

void Logger::info(const std::string &message) const {
	log(Level::Info, message);
}

void Logger::warning(const std::string &message) const {
	log(Level::Warning, message);
}

void Logger::error(const std::string &message) const {
	log(Level::Error, message);
}
