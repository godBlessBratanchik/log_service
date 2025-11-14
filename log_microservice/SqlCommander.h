#pragma once

#include <string>
#include <libpq-fe.h>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

export
class SqlCommander
{
private:
	std::string host_;
	std::string port_;
	std::string dbname_;
	std::string user_;
	std::string password_;
	PGconn* conn_;
	bool is_connected_;

public:
	SqlCommander();
	~SqlCommander();

	void send_log(const std::string& log_time, const std::string& log_level,
		const std::string& message, const std::string& service_name, const std::string& function_method);

	json fetch_logs(const std::string& service_name, int limit);
	json get_system_status();
	bool is_healthy() const;

private:
	std::map<std::string, std::string> load_env(const std::string& filename);
	bool check_connection();
};
