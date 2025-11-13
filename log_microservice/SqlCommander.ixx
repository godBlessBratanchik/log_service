export module SqlCommander;

#include <string>
#include <libpq-fe.h>
#include <fstream>
#include <map>
#include <fmt/core.h>

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

public:
    SqlCommander();
    ~SqlCommander();

public:
    void send_log(const std::string& log_time, const std::string& log_level, const std::string& message,
        const std::string& service_name, const std::string& function_method);

private:
    std::map<std::string, std::string> load_env(const std::string& filename);
};

SqlCommander::SqlCommander() 
{
    // Заполнение данными бд
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
}

SqlCommander::~SqlCommander()
{
    if (conn_)
    {
        send_log(std::to_string(std::time(nullptr)), "1", 
            "База данных отключается", "log_microservice", "~SqlCommander");
        PQfinish(conn_);
    }
}

void SqlCommander::send_log(const std::string& log_time, const std::string& log_level, const std::string& message,
    const std::string& service_name, const std::string& function_method)
{
    static const char* sql =
        "INSERT INTO log (log_time, log_level, message, service_name, function_method) "
        "VALUES ($1, $2, $3, $4, $5)";

    const char* params[5] = {
        log_time.c_str(), log_level.c_str(), message.c_str(), 
        service_name.c_str(), function_method.c_str()
    };

    PGresult* res = PQexecParams(
        conn_,
        sql,
        5,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    PQclear(res);
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
