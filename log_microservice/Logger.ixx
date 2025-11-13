export module Logger;

import SqlCommander;

#include <mutex>
#include <libpq-fe.h>
#include <fmt/core.h>
#include <utility>
#include <ctime>

export
class Logger
{
public:
	SqlCommander sql;

public:
	Logger();
	~Logger() = default;

public:
	template <typename ... Args>
	void log(fmt::format_string<Args...> fmt_str, Args&&... args)
	{
		sql.send_log(std::to_string(std::time(nullptr)), std::forward<Args>(args)...);
	}
};

Logger::Logger() : sql() {}
