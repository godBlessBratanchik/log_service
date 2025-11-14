#include <iostream>
#include <memory>

#include "Logger.h";
#include "SqlCommander.h";
#include "APIGateway.h";
#include "HTTPServer.h";

int main()
{
	try {
		Logger logger;
		logger.info("Application started");

		HTTPServer http_server(8080);
		http_server.start();

		logger.info("HTTP server started successfully");

		http_server.register_handler("/logs", "GET",
			[&logger](const json& req) {
				return json{};
			});

		logger.info("Handlers registered");

		http_server.stop();
		logger.info("Application shutting down");
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}
