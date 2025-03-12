CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -I./include -O2 -I/usr/include -I./dotenv/include

SRC_DIR = src
AUTH_DIR = $(SRC_DIR)/Authentication
REST_DIR = $(SRC_DIR)/RestClient
WS_DIR = $(SRC_DIR)/WebsocketClient
OBJ_DIR = obj
BIN_DIR = bin
TRADE_DIR=$(SRC_DIR)/Trade
LOG_DIR=$(SRC_DIR)/Logger

SRC_FILES = $(AUTH_DIR)/Authorisation.cpp $(REST_DIR)/RestClient.cpp $(LOG_DIR)/Logger.cpp $(WS_DIR)/WebsocketClient.cpp $(TRADE_DIR)/Trade.cpp $(SRC_DIR)/main.cpp 
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))


OBJ_DIRS = $(sort $(dir $(OBJ_FILES)))


$(shell mkdir -p $(OBJ_DIRS) $(BIN_DIR))

LIBS= -lcurl -lfmt -lspdlog  -lpthread -lboost_system -lssl -lcrypto -lz

TARGET = $(BIN_DIR)/trading_app


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)


clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
