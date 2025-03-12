#ifndef TRADE_H
#define TRADE_H

#include <string>
#include <vector>
#include <set>
#include "RestClient.h"
#include "WebsocketClient.h"

// Enums for instruments for different products.
enum class SpotInstrument {
    BTC_SPOT,
    ETH_SPOT
};

enum class FuturesInstrument {
    BTC_PERPETUAL,
    ETH_PERPETUAL,
    USDC_PERPETUAL,
    USDT_PERPETUAL,
    BTC_FUTURES,
    ETH_FUTURES
};

enum class OptionsInstrument {
    BTC_OPTIONS,
    ETH_OPTIONS
};

// Enums for order sides and types.
enum class OrderSide {
    BUY,
    SELL
};

enum class OrderType {
    MARKET,
    LIMIT
};

enum class OptionType {
    CALL,
    PUT
};

// Order structures for each trading product.

// Spot Order structure.
struct SpotOrder {
    SpotInstrument instrument;
    OrderSide side;
    OrderType type;
    double amount;
    double price; // Used for limit orders.
};

// Futures Order structure.
struct FuturesOrder {
    FuturesInstrument instrument;
    OrderSide side;
    OrderType type;
    double amount;
    double price; // Used for limit orders.
    std::string expiryDate; // For futures, e.g., "2025-12-31".
};

// Options Order structure.
struct OptionsOrder {
    OptionsInstrument instrument;
    OrderSide side;
    OrderType type;
    double amount;
    double price; // Used for limit orders.
    std::string expiryDate;
    double strikePrice;
    OptionType optionType;
};

class Trade {
public:
    // Constructor accepts pointers to shared RestClient and WebsocketClient.
    Trade(RestClient* restClient, WebsocketClient* wsClient);
    ~Trade();

    // Order management functions.
    void placeSpotOrder(const SpotOrder& order);
    void placeFuturesOrder(const FuturesOrder& order);
    void placeOptionsOrder(const OptionsOrder& order);
    
    void cancelOrder(const std::string& orderId);
    void modifyOrder(const std::string& orderId, double newAmount, double newPrice);

    // WebSocket market data functions.
    // getOrderBook subscribes to order book updates for a given symbol.
    void getOrderBook(const std::string& instrument);
    // viewCurrentPositions requests current positions (assumes a currency, e.g., "BTC").
    void viewCurrentPositions();
    // Subscription functions.
    void subscribeToOrderBook(const std::vector<std::string>& instruments);
    void subscribeToMarketTrades(const std::vector<std::string>& instruments);

    // Utility conversion functions.
    static std::string spotInstrumentToString(SpotInstrument instrument);
    static std::string futuresInstrumentToString(FuturesInstrument instrument);
    static std::string optionsInstrumentToString(OptionsInstrument instrument);
    static std::string orderSideToString(OrderSide side);
    static std::string orderTypeToString(OrderType type);
    static std::string optionTypeToString(OptionType type);

private:
    RestClient* restClient;
    WebsocketClient* wsClient;
};

#endif // TRADE_H

