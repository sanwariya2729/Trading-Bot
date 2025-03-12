#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <chrono>

extern std::shared_ptr<spdlog::logger> systemLogger;
extern std::shared_ptr<spdlog::logger> orderLogger;
extern std::shared_ptr<spdlog::logger> latencyLogger;
extern std::shared_ptr<spdlog::logger> orderbook;
extern std::shared_ptr<spdlog::logger> markettrade;
extern std::shared_ptr<spdlog::logger> positions;
extern std::shared_ptr<spdlog::logger> dump;


void initLogger();


#endif



