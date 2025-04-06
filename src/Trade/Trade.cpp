#include "trade.h"
#include "spdlog/spdlog.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

using json = nlohmann::json;

// ------------------ Utility Functions ------------------ //

std::string Trade::spotInstrumentToString(SpotInstrument instrument) {
    switch (instrument) {
        case SpotInstrument::BTC_SPOT: return "BTC-PERPETUAL";
        case SpotInstrument::ETH_SPOT: return "ETH-PERPETUAL";
        default: return "UNKNOWN_SPOT";
    }
}

std::string Trade::futuresInstrumentToString(FuturesInstrument instrument) {
    switch (instrument) {
        case FuturesInstrument::BTC_PERPETUAL: return "BTC-PERPETUAL";
        case FuturesInstrument::ETH_PERPETUAL: return "ETH-PERPETUAL";
        case FuturesInstrument::USDC_PERPETUAL: return "USDC-PERPETUAL";
        case FuturesInstrument::USDT_PERPETUAL: return "USDT-PERPETUAL";
        case FuturesInstrument::BTC_FUTURES: return "BTC-FUTURES";
        case FuturesInstrument::ETH_FUTURES: return "ETH-FUTURES";
        default: return "UNKNOWN_FUTURES";
    }
}

std::string Trade::optionsInstrumentToString(OptionsInstrument instrument) {
    switch (instrument) {
        case OptionsInstrument::BTC_OPTIONS: return "BTC-OPTIONS";
        case OptionsInstrument::ETH_OPTIONS: return "ETH-OPTIONS";
        default: return "UNKNOWN_OPTIONS";
    }
}

std::string Trade::orderSideToString(OrderSide side) {
    return side == OrderSide::BUY ? "buy" : "sell";
}

std::string Trade::orderTypeToString(OrderType type) {
    return type == OrderType::MARKET ? "market" : "limit";
}

std::string Trade::optionTypeToString(OptionType type) {
    return type == OptionType::CALL ? "call" : "put";
}

// ------------------ Constructor / Destructor ------------------ //

Trade::Trade(RestClient* restClient, WebsocketClient* wsClient)
    : restClient(restClient), wsClient(wsClient) {
    
}

Trade::~Trade() {
    wsClient->stop();
}

// ------------------ Order Management Functions ------------------ //

void Trade::placeSpotOrder(const SpotOrder& order) {
    json payload = {
        {"instrument", spotInstrumentToString(order.instrument)},
        {"side", orderSideToString(order.side)},
        {"type", orderTypeToString(order.type)},
        {"amount", order.amount},
        {"price", order.price}
    };
    spdlog::info("Placing Spot Order: {}", payload.dump());
    std::string response = restClient->placeOrder(spotInstrumentToString(order.instrument),
                                                    order.amount,
                                                    orderSideToString(order.side),
                                                    orderTypeToString(order.type),
                                                    order.price,"",0.0,"");
    spdlog::info("Spot Order Response: {}", response);
}

void Trade::placeFuturesOrder(const FuturesOrder& order) {
    json payload = {
        {"instrument", futuresInstrumentToString(order.instrument)},
        {"side", orderSideToString(order.side)},
        {"type", orderTypeToString(order.type)},
        {"amount", order.amount},
        {"price", order.price},
        {"expiry_date", order.expiryDate}
    };
    spdlog::info("Placing Futures Order: {}", payload.dump());
    std::string response = restClient->placeOrder(futuresInstrumentToString(order.instrument),
                                                    order.amount,
                                                    orderSideToString(order.side),
                                                    orderTypeToString(order.type),
                                                    order.price,order.expiryDate,0.0,"");
    spdlog::info("Futures Order Response: {}", response);
}

void Trade::placeOptionsOrder(const OptionsOrder& order) {
    json payload = {
        {"instrument", optionsInstrumentToString(order.instrument)},
        {"side", orderSideToString(order.side)},
        {"type", orderTypeToString(order.type)},
        {"amount", order.amount},
        {"price", order.price},
        {"expiry_date", order.expiryDate},
        {"strike_price", order.strikePrice},
        {"option_type", optionTypeToString(order.optionType)}
    };
    
    std::string response = restClient->placeOrder(optionsInstrumentToString(order.instrument),
                                                    order.amount,
                                                    orderSideToString(order.side),
                                                    orderTypeToString(order.type),
                                                    order.price,
                                                    order.expiryDate,
                                                    order.strikePrice,
                                                    optionTypeToString(order.optionType));
    
}

void Trade::cancelOrder(const std::string& orderId) {
    spdlog::info("Cancelling Order with ID: {}", orderId);
    std::string response = restClient->cancelOrder(orderId);
    spdlog::info("Cancel Order Response: {}", response);
}

void Trade::modifyOrder(const std::string& orderId, double newAmount, double newPrice) {
    spdlog::info("Modifying Order ID: {} to new amount: {} and new price: {}", orderId, newAmount, newPrice);
    std::string response = restClient->modifyOrder(orderId, newAmount, newPrice);
    spdlog::info("Modify Order Response: {}", response);
}

// ------------------ WebSocket Market Data Functions ------------------ //

void Trade::getOrderBook(const std::string& instrument) {
    // Wrap instrument in a vector and subscribe via the WebSocket client.
    std::vector<std::string> instruments = { instrument };
    wsClient->subscribeToOrderBook(instruments);
    spdlog::info("Subscribed to order book for instrument: {}", instrument);
}

void Trade::viewCurrentPositions() {
    // Request positions via WebSocket (assuming currency is BTC for this example).
    //hardcoded it.. could be modified.
    wsClient->requestCurrentPositions("BTC");
    spdlog::info("Requested current positions for BTC");
}

void Trade::subscribeToOrderBook(const std::vector<std::string>& instruments) {
    wsClient->subscribeToOrderBook(instruments);
    spdlog::info("Subscribed to order book for {} instruments", instruments.size());
}

void Trade::subscribeToMarketTrades(const std::vector<std::string>& instruments) {
    wsClient->subscribeToMarketTrades(instruments);
    spdlog::info("Subscribed to market trades for {} instruments", instruments.size());
}

