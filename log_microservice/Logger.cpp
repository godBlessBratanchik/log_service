#include "Logger.h";

#include <iostream>

Logger::Logger() : sql_() {}

std::string Logger::get_current_timestamp() const
{
	auto now = std::time(nullptr);
	auto tm = *std::localtime(&now);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}

void Logger::debug(fmt::format_string<> fmt_str)
{
	log("DEBUG", fmt_str);
}

void Logger::info(fmt::format_string<> fmt_str)
{
	log("INFO", fmt_str);
}

void Logger::warn(fmt::format_string<> fmt_str)
{
	log("WARN", fmt_str);
}

void Logger::error(fmt::format_string<> fmt_str)
{
	log("ERROR", fmt_str);
}
