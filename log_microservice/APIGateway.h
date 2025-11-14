#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>

using json = nlohmann::json;

export
class KafkaConnection
{
private:
	std::string brokers_;
	std::string group_id_;
	std::string topic_name_;

	RdKafka::KafkaConsumer* consumer_;
	RdKafka::Conf* conf_;
	RdKafka::Conf* tconf_;

	std::string error_string_;

public:
	KafkaConnection(const std::string& brokers, const std::string& group_id, const std::string& topic_name);
	~KafkaConnection();

	void consume_loop();
	RdKafka::Message* consume_message(int timeout_ms);
	bool is_connected() const;
	std::string get_last_error() const;

private:
	void handle_message(RdKafka::Message* msg);
};