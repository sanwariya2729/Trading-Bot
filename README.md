

# CLI TRADE EXECUTION SYSTEM

## PROJECT DESCRIPTION:-A C++ trading system that supports order placement, cancellation, modification, and live market data streaming over WebSockets.

## FEATURES:-
	-places orders(futures,options,spot)
	-cancle and modify orders.
	-Real time market data streaming via Websockets.
	-Order book retrival
	-get positions data.
	
	
## DEPENDENCIES:-
	make -build automation tool
		install with:
			sudo apt install make
	curl-http request to simulate REST behavior
		install with:
			sudo apt install libcurl14-openssl-dev
	spdlog-external logging library to log the events(behind the scenes)
		install with:
			sudo apt install libspdlog-dev
	fmt-for string formating used by spdlog
		install with:
			sudo apt install libfmt-dev
	pthread-for multithreading
		install with:
			sudo apt install libc6-dev
	boost-for websocket connections
		install with:
			sudo apt install libboot-system-dev
	openssl-for secure connections used in curl and websocket
		install with:
			sudo apt install libssl-dev
	lib-for compression support
		install with:
			sudo apt install zlib-dev
	(installations are for linux system)
	
## BUILD:-
	to build the project run
		 make clean && make

## HOW TO USE:-
	after running the above command in the terminal 
		you will find the binary in bin/ named trading_app
			after that run ./bin/trading_app
			


		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
