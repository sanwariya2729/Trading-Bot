#include <iostream>
#include "Logger.h"

std::shared_ptr<spdlog::logger> systemLogger; 
std::shared_ptr<spdlog::logger> orderLogger;
std::shared_ptr<spdlog::logger> latencyLogger;
std::shared_ptr<spdlog::logger> orderbook;
std::shared_ptr<spdlog::logger> markettrade;
std::shared_ptr<spdlog::logger> positions;
std::shared_ptr<spdlog::logger> dump;


void initLogger(){
	try{
		systemLogger=spdlog::get("system_logger");
		if(!systemLogger){
			systemLogger=spdlog::basic_logger_mt("system_logger","logs/system.log",true);
			
		}
		
		orderLogger=spdlog::get("order_logger");
		if(!orderLogger){
			orderLogger=spdlog::basic_logger_mt("order_logger","logs/orders.log",true);
		}
		
		latencyLogger=spdlog::get("latency_logger");
		if(!latencyLogger){
			latencyLogger=spdlog::basic_logger_mt("latency_logger","logs/latency.log",true);
		}
			
		orderbook=spdlog::get("orderbook");
		if(!orderbook){
			orderbook=spdlog::basic_logger_mt("orderbook","logs/orderbook.log",true);
		}
		
		markettrade=spdlog::get("market_trade");
		if(!markettrade){
			markettrade=spdlog::basic_logger_mt("market_trade","logs/markettrade.log",true);
		}
		
		positions=spdlog::get("positions");
		if(!positions){
			positions=spdlog::basic_logger_mt("positions","logs/positions.log",true);
		}
		
		dump=spdlog::get("dump");
		if(!dump){
			dump=spdlog::basic_logger_mt("dump","logs/dump.log",true);
		}
		
		spdlog::flush_every(std::chrono::seconds(1));

		
		
	}catch(const spdlog::spdlog_ex& ex){
		std::cerr<<"Logger intitialisation failed"<<ex.what()<<std::endl;
	}


}
