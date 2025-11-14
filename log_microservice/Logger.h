#pragma once

#include "SqlCommander.h";

#include <mutex>
#include <string>
#include <fmt/core.h>
#include <utility>
#include <ctime>
#include <iomanip>
#include <sstream>

export
class Logger
{
private:
	SqlCommander sql_;
	std::mutex log_mutex_;

public:
	Logger();
	~Logger() = default;

	template <typename... Args>
	void log(const std::string& level, fmt::format_string<Args...> fmt_str, Args&&... args)
	{
		std::lock_guard<std::mutex> lock(log_mutex_);
		auto timestamp = get_current_timestamp();
		auto formatted_msg = fmt::format(fmt_str, std::forward<Args>(args)...);
		sql_.send_log(timestamp, level, formatted_msg, "log_microservice", "log");
	}

	void debug(fmt::format_string<> fmt_str);
	void info(fmt::format_string<> fmt_str);
	void warn(fmt::format_string<> fmt_str);
	void error(fmt::format_string<> fmt_str);

private:
	std::string get_current_timestamp() const;
};
