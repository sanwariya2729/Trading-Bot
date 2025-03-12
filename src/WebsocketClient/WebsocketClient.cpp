#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include "WebsocketClient.h"
#include <iostream>
#include <sstream>
#include <boost/asio/ssl/context.hpp>
#include <boost/bind/bind.hpp>

// TLS initialization function.
std::shared_ptr<boost::asio::ssl::context> WebsocketClient::on_tls_init() {
    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    ctx->set_default_verify_paths();
    return ctx;
}

WebsocketClient::WebsocketClient(Authorization* auth)
    : running(false),  auth(auth)
{
	initLogger();
    // Initialize the ASIO transport.
    wsClient.init_asio();
    wsClient.clear_access_channels(websocketpp::log::alevel::all);
	wsClient.set_error_channels(websocketpp::log::elevel::rerror); // Only report runtime errors.


    // Set TLS initialization handler.
    wsClient.set_tls_init_handler(boost::bind(&WebsocketClient::on_tls_init));

    // Set message handler.
    wsClient.set_message_handler([this](connection_hdl /*hdl*/, WebsocketppClient::message_ptr msg) {
        auto start_time = std::chrono::high_resolution_clock::now();
        try {
            auto jsonMessage = json::parse(msg->get_payload());
             
            // Check if the message contains subscription data.
            if (jsonMessage.contains("params") && jsonMessage["params"].contains("channel")) {
                std::string channel = jsonMessage["params"]["channel"];
                // Print based on channel type.
                if (channel.find("book.") != std::string::npos) {
                    orderbook->info("[OrderBook] {}\n\n",jsonMessage.dump(4));
                } else if (channel.find("ticker.") != std::string::npos) {
                    markettrade->info("[MarketTrade] {}\n\n",
                              jsonMessage.dump(4));
                } else {
                    dump->info("[Other] {}", channel ,":\n {}",
                              jsonMessage.dump(4), "\n\n");
                }
            } else if (jsonMessage.contains("result")) {
                // Handle responses such as positions data or authentication.
                if (jsonMessage["id"] == 3) {
                    positions->info("[Positions] {}\n\n",jsonMessage["result"].dump(4));
                } else if (jsonMessage["id"] == 0) {
                    //dont want acces tokens in any log file 
                }
            } else {
               
                dump->info("[Raw Message]: {}\n" ,jsonMessage.dump(4));
            }
        } catch (const std::exception& e) {
            systemLogger->error("Error parsing message: {}",e.what());
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto processing_latency = std::chrono::duration<double, std::micro>(end_time - start_time).count();
        latencyLogger->info("Processed message in {} (microseconds)",processing_latency);
    });

    // Set open handler.
    wsClient.set_open_handler([this](connection_hdl /*hdl*/) {
        systemLogger->info( "[Websocket Client] Connected to Deribit WebSocket.");
        reconnectAttempts = 0;
        wsAuthenticate();
    });

    // Set close handler.
    wsClient.set_close_handler([this](connection_hdl /*hdl*/) {
        systemLogger->error("[Websocket Client] WebSocket closed. Attempting to reconnect...");
        wsConnection.reset();
        attemptReconnect();
    });

    // Set fail handler.
    wsClient.set_fail_handler([this](connection_hdl /*hdl*/) {
        systemLogger->error( " [Websocket Client] WebSocket connection failed. Attempting to reconnect...");
        wsConnection.reset();
        attemptReconnect();
    });
}

WebsocketClient::~WebsocketClient() {
    stop();
}

void WebsocketClient::start() {
    if (running.load()) return;
    running.store(true);
    wsThread = std::make_unique<std::thread>(&WebsocketClient::websocketLoop, this);
    systemLogger->info(" [Websocket Client] Starting WebSocket client.");
}

void WebsocketClient::stop() {
    running.store(false);
    wsClient.stop();
    if (wsThread && wsThread->joinable()) {
        wsThread->join();
    }
    systemLogger->info("[Websocket Client] WebSocket client stopped.");
}

void WebsocketClient::websocketLoop() {
    websocketpp::lib::error_code ec;
    auto con = wsClient.get_connection("wss://test.deribit.com/ws/api/v2", ec);
    if (ec) {
        systemLogger->error("[Websocket Client] Websocket Connection error: {}", ec.message());
        return;
    }
    wsConnection = con->get_handle();
    wsClient.connect(con);
    // The run() call will block until stop() is called.
    wsClient.run();
}

void WebsocketClient::sendMessage(const json& message) {
    if (!wsConnection.expired()) {
        std::string msgStr = message.dump();
        websocketpp::lib::error_code ec;
        wsClient.send(wsConnection, msgStr, websocketpp::frame::opcode::text, ec);
        if (ec) {
            systemLogger->error("[Websocket Client] error while sending payload to server: {}" ,ec.message());
        } else {
            orderLogger->info("[Websocket Client] msg sent  through websocket.");
        }
    } else {
       	systemLogger->error("[Websocket Client] WebSocket not connected. Cannot send message.");
    }
}

void WebsocketClient::subscribeToOrderBook(const std::vector<std::string>& instruments) {
    // Get the token from the authentication module.
    std::string token = auth->getAccessToken();
 
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "public/subscribe"},
        {"params", {
            {"channels", json::array()},
            {"token", token}
        }}
    };

    for (const auto& instrument : instruments) {
        std::string channel = "book." + instrument+".raw" ;
        if (subscribedChannels.find(channel) == subscribedChannels.end()) {
            request["params"]["channels"].push_back(channel);
            subscribedChannels.insert(channel);
        }
    }
    if (!request["params"]["channels"].empty()) {
    	auto start=std::chrono::high_resolution_clock::now();
    	sendMessage(request);
    	auto end=std::chrono::high_resolution_clock::now();
    	auto msg_delay = std::chrono::duration<double, std::micro>(end - start).count();
        latencyLogger->info(" time taken to sent request to subscribe to  order book recieve a response in {} (microseconds)",msg_delay);  
        systemLogger->info("[Websocket Client] subscribed to order book in instuments.");  
    }
}

void WebsocketClient::subscribeToMarketTrades(const std::vector<std::string>& instruments) {
    // Get the token from the authentication module.
    std::string token = auth->getAccessToken();
    std::cout<<token;

    json request = {
        {"jsonrpc", "2.0"},
        {"id", 2},
        {"method", "public/subscribe"},
        {"params", {
            {"channels", json::array()},
            {"token", token}
        }}
    };

    for (const auto& instrument : instruments) {
        std::string channel = "ticker." + instrument + ".raw";
        if (subscribedChannels.find(channel) == subscribedChannels.end()) {
            request["params"]["channels"].push_back(channel);
            subscribedChannels.insert(channel);
        }
    }
    if (!request["params"]["channels"].empty()) {
    	auto start=std::chrono::high_resolution_clock::now();
        sendMessage(request);
        auto end=std::chrono::high_resolution_clock::now();
        auto msg_delay = std::chrono::duration<double, std::micro>(end - start).count();
        latencyLogger->info(" time taken to sent request to get market trades and recieve a response in {} (microseconds)",msg_delay);
        systemLogger->info("[Websocket Client] Subscribed to market data for {} instrument(s).", instruments.size());


    } else {
        systemLogger->error("[Websocket Client] No new instruments to subscribe for market data.");
    }
}

void WebsocketClient::requestCurrentPositions(const std::string& currency) {
    // Get the token from the authentication module.
    std::string token = auth->getAccessToken();
    
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 3},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", currency},
            {"kind", "future"},
            {"token", token}
        }}
    };
	auto start=std::chrono::high_resolution_clock::now();
    sendMessage(request);
    auto end=std::chrono::high_resolution_clock::now();
    auto msg_delay = std::chrono::duration<double, std::micro>(end - start).count();
    latencyLogger->info(" time taken to send positions info and recieve a response is {} (microseconds)",msg_delay);
    orderLogger->info("[Websocket Client] Requested current positions for currency: {}",currency);
}

void WebsocketClient::attemptReconnect() {
    if (reconnectAttempts >= maxReconnectAttempts) {
        systemLogger->error("[Websocket Client] Maximum reconnection attempts reached. Giving up."); 
        return;
    }
    
    int delay = std::min(1 << reconnectAttempts, maxReconnectDelay); 
    systemLogger->info("[Websocket Client] Reconnecting in {}", delay, " seconds..." ); std::this_thread::sleep_for(std::chrono::seconds(delay));
    ++reconnectAttempts;
    websocketLoop();
}


void WebsocketClient::wsAuthenticate() {

    json authMessage = {
        {"jsonrpc", "2.0"},
        {"id", 0}, 
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", auth->getClientId()},      
            {"client_secret", auth->getClientSecret()},
            {"scope", "read_write"}
        }}
    };
    sendMessage(authMessage);
    std::cout << "Sent WebSocket authentication message." << std::endl;
}



