#include "SqlCommander.h";

#include <fstream>
#include <fmt/core.h>
#include <iostream>
#include <sstream>

SqlCommander::SqlCommander() : conn_(nullptr), is_connected_(false)
{
	auto env = load_env("config.env");

	host_ = env.at("HOST");
	port_ = env.at("PORT");
	dbname_ = env.at("DBNAME");
	user_ = env.at("USER");
	password_ = env.at("PASSWORD");

	std::string conn_str = fmt::format(
		"host={} port={} dbname={} user={} password={}",
		host_, port_, dbname_, user_, password_);

	conn_ = PQconnectdb(conn_str.c_str());

	if (PQstatus(conn_) != CONNECTION_OK) 
	{
		std::cerr << "Connection failed: " << PQerrorMessage(conn_) << "\n";
		PQfinish(conn_);
		conn_ = nullptr;
		is_connected_ = false;
	}
	else 
	{
		is_connected_ = true;
	}
}

SqlCommander::~SqlCommander()
{
	if (conn_ && is_connected_) 
	{
		send_log("DB_SHUTDOWN", "1", "Database connection closed", "db_microservice", "~SqlCommander");
		PQfinish(conn_);
	}
}

void SqlCommander::send_log(const std::string& log_time, const std::string& log_level,
	const std::string& message, const std::string& service_name, const std::string& function_method)
{
	if (!check_connection()) return;

	static const char* sql =
		"INSERT INTO log (log_time, log_level, message, service_name, function_method) "
		"VALUES ($1, $2, $3, $4, $5)";

	const char* params[5] = {
		log_time.c_str(), log_level.c_str(), message.c_str(),
		service_name.c_str(), function_method.c_str()
	};

	PGresult* res = PQexecParams(conn_, sql, 5, nullptr, params, nullptr, nullptr, 0);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) 
	{
		std::cerr << "Insert failed: " << PQerrorMessage(conn_) << "\n";
	}

	PQclear(res);
}

json SqlCommander::fetch_logs(const std::string& service_name, int limit)
{
	if (!check_connection()) return json::array();

	std::string sql = fmt::format(
		"SELECT * FROM log WHERE service_name = $1 ORDER BY log_time DESC LIMIT $2");

	const char* params[2] = { service_name.c_str(), std::to_string(limit).c_str() };

	PGresult* res = PQexecParams(conn_, sql.c_str(), 2, nullptr, params, nullptr, nullptr, 0);

	json result = json::array();

	if (PQresultStatus(res) == PGRES_TUPLES_OK) 
	{
		int rows = PQntuples(res);
		for (int i = 0; i < rows; i++) 
		{
			json log_entry = {
				{"log_time", PQgetvalue(res, i, 0)},
				{"log_level", PQgetvalue(res, i, 1)},
				{"message", PQgetvalue(res, i, 2)},
				{"service_name", PQgetvalue(res, i, 3)},
				{"function_method", PQgetvalue(res, i, 4)}
			};
			result.push_back(log_entry);
		}
	}

	PQclear(res);
	return result;
}

json SqlCommander::get_system_status()
{
	return 
	{
		{"status", is_connected_ ? "healthy" : "unhealthy"},
		{"database", dbname_},
		{"host", host_},
		{"timestamp", std::time(nullptr)}
	};
}

bool SqlCommander::is_healthy() const
{
	return is_connected_ && conn_ != nullptr && PQstatus(conn_) == CONNECTION_OK;
}

bool SqlCommander::check_connection()
{
	if (!is_connected_ || !conn_) return false;

	if (PQstatus(conn_) != CONNECTION_OK) 
	{
		std::cerr << "Database connection lost\n";
		is_connected_ = false;
		return false;
	}

	return true;
}

std::map<std::string, std::string> SqlCommander::load_env(const std::string& filename)
{
	std::map<std::string, std::string> env;
	std::ifstream file(filename);
	std::string line;

	while (std::getline(file, line)) 
	{
		size_t eq_pos = line.find('=');

		if (eq_pos != std::string::npos) 
		{
			std::string key = line.substr(0, eq_pos);
			std::string value = line.substr(eq_pos + 1);

			if (!key.empty() && !value.empty()) 
			{
				env[key] = value;
			}
		}
	}

	return env;
}
