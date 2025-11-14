#include "HTTPServer.h";

#include <iostream>
#include <sstream>

HTTPServer::HTTPServer(int port) : port_(port), running_(false) {}

HTTPServer::~HTTPServer()
{
	stop();
}

void HTTPServer::start()
{
	running_ = true;
	std::cout << "HTTP Server started on port " << port_ << "\n";
}

void HTTPServer::stop()
{
	running_ = false;
	std::cout << "HTTP Server stopped\n";
}

void HTTPServer::register_handler(const std::string& path, const std::string& method,
	std::function<json(const json&)> handler)
{
	std::string key = method + " " + path;
	handlers_[key] = handler;
}

json HTTPServer::handle_get_logs(const json& request)
{
	return 
	{
		{"status", "success"},
		{"data", json::array()}
	};
}

json HTTPServer::handle_get_status(const json& request)
{
	return 
	{
		{"status", "ok"},
		{"timestamp", std::time(nullptr)}
	};
}

json HTTPServer::handle_create_log(const json& request)
{
	if (!request.contains("message")) 
	{
		return 
		{
			{"status", "error"},
			{"message", "Missing required field: message"}
		};
	}

	return 
	{
		{"status", "success"},
		{"message", "Log entry created"}
	};
}
