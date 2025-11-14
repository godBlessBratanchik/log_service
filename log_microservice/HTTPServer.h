#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <memory>

using json = nlohmann::json;

export
class HTTPServer
{
public:
	HTTPServer(int port);
	~HTTPServer();

	void start();
	void stop();
	void register_handler(const std::string& path, const std::string& method,
		std::function<json(const json&)> handler);

	json handle_get_logs(const json& request);
	json handle_get_status(const json& request);
	json handle_create_log(const json& request);

private:
	int port_;
	bool running_;
	std::map<std::string, std::function<json(const json&)>> handlers_;
};
