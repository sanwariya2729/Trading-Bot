#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <zlib.h>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <set>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include "Authorisation.h" 
#include "Logger.h"


// Use nlohmann::json for JSON handling.
using json = nlohmann::json;
using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_tls_client> WebsocketppClient;

class WebsocketClient {
public:
    // Pass an Authorization pointer so the client can fetch a valid token.
    explicit WebsocketClient(Authorization* auth);
    ~WebsocketClient();

    // Start and stop the client event loop.
    void start();
    void stop();
    
    // Methods to send requests.
    void subscribeToOrderBook(const std::vector<std::string>& instruments);
    void subscribeToMarketTrades(const std::vector<std::string>& instruments);
    void requestCurrentPositions(const std::string& currency);

private:
    std::set<std::string> subscribedChannels;
    std::atomic<bool> running;
    std::unique_ptr<std::thread> wsThread;
    WebsocketppClient wsClient;
    websocketpp::connection_hdl wsConnection;

    // Pointer to our authentication object.
    Authorization* auth;

    // Internal methods.
    void websocketLoop();
    void sendMessage(const json& message);
    void attemptReconnect();

    int reconnectAttempts=0;
    const int maxReconnectAttempts = 5;
    const int maxReconnectDelay = 30; // seconds

    // TLS initialization callback.
    static std::shared_ptr<boost::asio::ssl::context> on_tls_init();
    void wsAuthenticate();
    
};

#endif // WEBSOCKETCLIENT_H

