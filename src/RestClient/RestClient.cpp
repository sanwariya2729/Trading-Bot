#include "RestClient.h"
#include <iostream>
#include <curl/curl.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/sinks/basic_file_sink.h>


using json = nlohmann::json;

// Callback function for libcurl to collect response data.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* response = reinterpret_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    response->append(reinterpret_cast<char*>(contents), totalSize);
    return totalSize;
}

RestClient::RestClient(Authorization* auth)
    : auth(auth)
{
    curl_global_init(CURL_GLOBAL_ALL);
    initLogger();
    systemLogger->info("RestClient initialized.");
}

RestClient::~RestClient() {
    curl_global_cleanup();
    systemLogger->info("RestClient shutting down.");
}

std::string RestClient::httpPost(const std::string& url, const std::string& jsonPayload) {
    CURL* curl = curl_easy_init();  
    std::string response;
    if (curl) {
        struct curl_slist* headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + auth->getAccessToken();
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            systemLogger->error("[RestClient] HTTP POST failed: {}", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return response;
}


std::string RestClient::placeOrder(const std::string& instrument, double amount, 
                                   const std::string& side, const std::string& orderType, double price,
                                   const std::string& expiryDate , double strikePrice , 
                                   const std::string& optionType ) {
                            
    
    std::string url = (side == "buy") ? "https://test.deribit.com/api/v2/private/buy" 
                                      : "https://test.deribit.com/api/v2/private/sell";
    if(url.empty()){
    	std::cerr<<"ENV variables not loaded properly"<<std::endl;
    }

  
    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", side == "buy" ? "private/buy" : "private/sell"},
        {"params", {
            {"instrument_name", instrument},
            {"amount", amount},
            {"type", orderType}
        }}
    };

   
    if (orderType == "limit") {
        payload["params"]["price"] = price;
    }

  
    if (!expiryDate.empty()) {
        payload["params"]["expiry_date"] = expiryDate;
    }

   
    if (strikePrice != 0.0) {
        payload["params"]["strike_price"] = strikePrice;
    }
    if (!optionType.empty()) {
        payload["params"]["option_type"] = optionType;
    }

    orderLogger->info("Placing {} order for {}: Amount: {}, Type: {}", side, instrument, amount, orderType);
    auto start = std::chrono::high_resolution_clock::now();
    std::string response = httpPost(url, payload.dump());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    latencyLogger->info("Time taken to place order and receive a response (in microseconds): {}", duration);

    
    try {
        auto j = json::parse(response);
        orderLogger->info("ORDER INFO: {}", j.dump(4));  
        if (j.contains("result") && j["result"].contains("order")) {
            std::string orderId = j["result"]["order"].value("order_id", "ORDER_FAILED");
            return orderId;
        } else if (j.contains("error")) {
            systemLogger->error("Order failed: {}", j["error"].dump());
            return "ORDER_FAILED";
        }
    } catch (json::exception& e) {
        systemLogger->error("JSON parsing error: {} | Raw response: {}", e.what(), response);
        return "JSON_ERROR";
    }
    return "UNKNOWN_ERROR";
}




std::string RestClient::cancelOrder(const std::string& orderId) {
	
    std::string url = "https://test.deribit.com/api/v2/private/cancel";
    if(url.empty()){
    	std::cerr<<"ENV variables not loaded properly"<<std::endl;
    }
    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/cancel"},
        {"params", { {"order_id", orderId} }}
    };

    orderLogger->info("Canceling order: {}", orderId);
    auto start=std::chrono::high_resolution_clock::now();
    std::string response = httpPost(url, payload.dump());
    auto end=std::chrono::high_resolution_clock::now();
    auto duration=std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    latencyLogger->info("Time taken to cancle an order and recieve a conformation (in microseconds) {}",duration);
    
    
    try {
        auto j = json::parse(response);
        orderLogger->info("CANCLE ORDER INFO {}",j.dump(4));
        if (j.contains("result")) {
            return "ORDER_CANCELED";
        } else if (j.contains("error")) {
            systemLogger->error("Cancel failed: {}", j["error"].dump());
            return "CANCEL_FAILED";
        }
    } catch (json::exception& e) {
        systemLogger->error("JSON parsing error: {}", e.what());
        return "JSON_ERROR";
    }
    return "UNKNOWN_ERROR";
}

std::string RestClient::modifyOrder(const std::string& orderId, double newAmount, double newPrice) {

    std::string url = "https://test.deribit.com/api/v2/private/edit";
    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/edit"},
        {"params", {
            {"order_id", orderId},
            {"amount", newAmount},
            {"price", newPrice}
        }}
    };

    orderLogger->info("Modifying order {}: New Amount: {}, New Price: {}", orderId, newAmount, newPrice);
    auto start=std::chrono::high_resolution_clock::now();
    std::string response = httpPost(url, payload.dump());
    auto end=std::chrono::high_resolution_clock::now();
    auto duration=std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
    latencyLogger->info("Time taken to modify the order and recieve a conformation back (in microsecnds) {}",duration);
    
    try {
        auto j = json::parse(response);
        orderLogger->info("MODIFY ORDER INFO {}",j.dump(4));
        if (j.contains("result")) {
            return j["result"].value("order_id", "MODIFY_FAILED");
        } else if (j.contains("error")) {
            systemLogger->error("Modify failed: {}", j["error"].dump());
            return "MODIFY_FAILED";
        }
    } catch (json::exception& e) {
        systemLogger->error("JSON parsing error: {}", e.what());
        return "JSON_ERROR";
    }
    return "UNKNOWN_ERROR";
}


