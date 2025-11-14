#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include "APIGateway.h"
#include "HTTPServer.h"
#include "Logger.h"
#include "SqlCommander.h"
#include <nlohmann/json.hpp>

using ::testing::Return;
using ::testing::_;

using json = nlohmann::json;

class MockSqlCommander : public SqlCommander
{
public:
    MOCK_METHOD(void, send_log, (const std::string&, const std::string&, const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, is_healthy, (), (const, override));
};

class LoggerWithDI
{
public:
    explicit LoggerWithDI(SqlCommander* commander) : sql_commander(commander) {}
    void info(const std::string& msg)
    {
        if (sql_commander)
            sql_commander->send_log("INFO", msg, "", "", "");
    }
private:
    SqlCommander* sql_commander;
};

TEST(LoggerUnitTest, InfoLog)
{
    MockSqlCommander sqlMock;
    EXPECT_CALL(sqlMock, send_log(_, _, _, _, _)).Times(1);

    LoggerWithDI logger(&sqlMock);
    logger.info("Test info message");
}

TEST(KafkaUnitTest, ConnectSubscribe)
{
    KafkaConnection conn("localhost:9092", "group", "topic");
    ASSERT_TRUE(conn.is_connected());
}

TEST(HTTPServerComponentTest, StatusRequest)
{
    HTTPServer server(8080);
    server.start();
    auto res = server.handle_get_status(json{});
    EXPECT_EQ(res["status"], "ok");
    server.stop();
}

TEST(IntegrationTest, LoggerSqlCommander)
{
    SqlCommander sql;
    LoggerWithDI logger(&sql);
    logger.info("Integration test message");
    EXPECT_TRUE(sql.is_healthy());
}

TEST(IntegrationTest, KafkaHttpServer)
{
    KafkaConnection conn("kafka:9092", "testgroup", "testtopic");
    HTTPServer server(8080);

    server.start();
    server.register_handler("/logs", "GET", [](const json& req) {
        return json{ {"status", "success"} };
        });

    auto res = server.handle_get_logs(json{});
    EXPECT_EQ(res["status"], "success");

    server.stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
