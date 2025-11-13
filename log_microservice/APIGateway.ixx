export module APIGateway;

#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>
#include <iostream>

export
class KafkaConnection : public RdKafka::EventCb, public RdKafka::ConsumeCb
{
private:
	std::string brokers_;
	std::string group_id_;
	std::string topic_name_;

	RdKafka::KafkaConsumer* consumer_;
	RdKafka::Conf* conf_;
	RdKafka::Conf* tconf_;

public:
	KafkaConnection(const std::string& brokers, const std::string& group_id, const std::string& topic_name);
	~KafkaConnection();

	void consume_loop();

	void event_cb(RdKafka::Event& event) override;
	void consume_cb(RdKafka::Message& message, void* opaque) override;
};

KafkaConnection::KafkaConnection(const std::string& brokers, const std::string& group_id, const std::string& topic_name)
	: brokers_(brokers), group_id_(group_id), topic_name_(topic_name), consumer_(nullptr), conf_(nullptr), tconf_(nullptr)
{
	std::string errstr;

	conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	tconf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

	conf_->set("bootstrap.servers", brokers_, errstr);
	conf_->set("group.id", group_id_, errstr);
	conf_->set("enable.auto.commit", "false", errstr);
	conf_->set("event_cb", (RdKafka::EventCb*)this, errstr);
	conf_->set("consume_cb", (RdKafka::EventCb*)(this), errstr);

	consumer_ = RdKafka::KafkaConsumer::create(conf_, errstr);

	if (!consumer_) 
	{
		std::cerr << "Failed to create consumer: " << errstr << "\n";
		throw std::runtime_error("Kafka consumer creation failed");
	}

	std::vector<std::string> topics = { topic_name_ };
	RdKafka::ErrorCode err = consumer_->subscribe(topics);

	if (err) 
	{
		std::cerr << "Failed to subscribe to topic: " << RdKafka::err2str(err) << "\n";
		throw std::runtime_error("Kafka subscription failed");
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

void KafkaConnection::event_cb(RdKafka::Event& event)
{
	switch (event.type()) {
	case RdKafka::Event::EVENT_ERROR:
		std::cerr << "Kafka event error: " << event.str() << "\n";
		break;
	default:
		break;
	}
}

void KafkaConnection::consume_cb(RdKafka::Message& message, void* opaque)
{
	switch (message.err()) {
	case RdKafka::ERR_NO_ERROR:
		std::cout << "Received message: " << static_cast<const char*>(message.payload()) << "\n";
		consumer_->commitSync(&message);
		break;
	case RdKafka::ERR__TIMED_OUT:
		break;
	case RdKafka::ERR__PARTITION_EOF:
		std::cout << "Reached end of partition" << "\n";
		break;
	default:
		std::cerr << "Kafka error: " << message.errstr() << "\n";
		break;
	}
}

void KafkaConnection::consume_loop()
{
	while (true) 
	{
		RdKafka::Message* msg = consumer_->consume(1000);

		if (msg) 
		{
			consume_cb(*msg, nullptr);
			delete msg;
		}
	}
}
