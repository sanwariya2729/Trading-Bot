#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <string>
#include "Logger.h"
#include "spdlog/spdlog.h"
#include "Authorisation.h"

class RestClient {
public:
    // Constructor: receives a pointer to the shared Authorization instance.
    explicit RestClient(Authorization* auth);
    ~RestClient();

    // Place an order.
    // 'side' should be either "buy" or "sell".
    // 'orderType' can be "market" or "limit". For limit orders, a price must be provided.
    std::string placeOrder(const std::string& instrument, double amount, 
                                   const std::string& side, const std::string& orderType, double price,
                                   const std::string& expiryDate = "", double strikePrice = 0.0, 
                                   const std::string& optionType = "");
	//std::string modifyOrder(const std::string& orderId, double newAmount, double newPrice,
      //                              const std::string& expiryDate = "", double strikePrice = 0.0,
        //                            const std::string& optionType = "");

    // Cancel an order using its order ID.
    std::string cancelOrder(const std::string& orderId);

    // Modify an existing order.
    std::string modifyOrder(const std::string& orderId, double newAmount, double newPrice);
    
    

private:
    Authorization* auth;  // Shared authorization object.
    
    // Helper function: perform a POST request.
    std::string httpPost(const std::string& url, const std::string& jsonPayload);
  
    
};

#endif 

