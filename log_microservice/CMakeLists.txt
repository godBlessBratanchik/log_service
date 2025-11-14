cmake_minimum_required(VERSION 3.20)
project(LoggingMicroservice)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

find_library(RDKAFKA_LIB rdkafka)
find_library(RDKAFKA_CPP_LIB rdkafka++)
find_path(RDKAFKA_INCLUDE_DIR librdkafka/rdkafkacpp.h)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${RDKAFKA_INCLUDE_DIR})

set(SOURCES
    src/log_microservice.cpp
    src/APIGateway.cpp
    src/HTTPServer.cpp
    src/Logger.cpp
    src/SqlCommander.cpp
)

add_executable(microservice ${SOURCES})

target_link_libraries(microservice
    PRIVATE
    PostgreSQL::PostgreSQL
    nlohmann_json::nlohmann_json
    fmt::fmt
    ${RDKAFKA_LIB}
    ${RDKAFKA_CPP_LIB}
    pthread
)