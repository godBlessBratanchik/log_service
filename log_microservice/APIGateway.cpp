#include "APIGateway.h"
#include <iostream>

KafkaConnection::KafkaConnection(const std::string& brokers, const std::string& group_id, const std::string& topic_name)
	: brokers_(brokers), group_id_(group_id), topic_name_(topic_name), consumer_(nullptr), conf_(nullptr), tconf_(nullptr)
{
	std::string errstr;

	conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	tconf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

	if (!conf_ || !tconf_) 
	{
		error_string_ = "Failed to create Kafka config";
		throw std::runtime_error(error_string_);
	}

	conf_->set("bootstrap.servers", brokers_, errstr);
	conf_->set("group.id", group_id_, errstr);
	conf_->set("enable.auto.commit", "false", errstr);

	consumer_ = RdKafka::KafkaConsumer::create(conf_, errstr);

	if (!consumer_) 
	{
		error_string_ = "Failed to create consumer: " + errstr;
		std::cerr << error_string_ << "\n";
		throw std::runtime_error(error_string_);
	}

	std::vector<std::string> topics = { topic_name_ };
	RdKafka::ErrorCode err = consumer_->subscribe(topics);

	if (err) 
	{
		error_string_ = "Failed to subscribe to topic: " + std::string(RdKafka::err2str(err));
		std::cerr << error_string_ << "\n";
		throw std::runtime_error(error_string_);
	}
}

KafkaConnection::~KafkaConnection()
{
	if (consumer_) 
	{
		consumer_->close();
		delete consumer_;
	}

	delete conf_;
	delete tconf_;
}

RdKafka::Message* KafkaConnection::consume_message(int timeout_ms)
{
	return consumer_->consume(timeout_ms);
}

void KafkaConnection::consume_loop()
{
	while (true) 
	{
		RdKafka::Message* msg = consume_message(1000);

		if (msg) 
		{
			handle_message(msg);
			delete msg;
		}
	}
}

void KafkaConnection::handle_message(RdKafka::Message* msg)
{
	switch (msg->err()) {
	case RdKafka::ERR_NO_ERROR:
		std::cout << "Received message: " << static_cast<const char*>(msg->payload()) << "\n";
		consumer_->commitSync(msg);
		break;
	case RdKafka::ERR__TIMED_OUT:
		break;
	case RdKafka::ERR__PARTITION_EOF:
		std::cout << "Reached end of partition\n";
		break;
	default:
		std::cerr << "Kafka error: " << msg->errstr() << "\n";
		break;
	}
}

bool KafkaConnection::is_connected() const
{
	return consumer_ != nullptr;
}

std::string KafkaConnection::get_last_error() const
{
	return error_string_;
}
