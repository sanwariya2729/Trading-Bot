#include "dotenv.h"
#include <iostream>
#include <string.h>
#include <chrono>
#include "trade.h"
#include "Authorisation.h"
#include "RestClient.h"
#include "WebsocketClient.h"
#include <vector>
#include <exception>
#include <chrono>


int main(){
	dotenv env(".env");
	
    std::string clientId = env.get("DERIBIT_CLIENT_ID");
    std::string clientSecret = env.get("DERIBIT_CLIENT_SECRET");
    if(clientId.empty() || clientSecret.empty()){
    	std::cerr<<"ENV variables not loaded properly"<<std::endl;
    }
    try {
        
        Authorization auth(clientId, clientSecret);
        RestClient restClient(&auth);
        WebsocketClient wsClient(&auth);
        wsClient.start();
        Trade trade(&restClient, &wsClient);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        bool running = true;
        while (running) {
            std::cout << "\n=== Trade Engine Menu ===\n";
            std::cout << "1. Place Spot Order\n";
            std::cout << "2. Place Futures Order\n";
            std::cout << "3. Place Options Order\n";
            std::cout << "4. Cancel Order\n";
            std::cout << "5. Modify Order\n";
            std::cout << "6. View Current Positions\n";
            std::cout << "7. Subscribe to Order Book (WebSocket)\n";
            std::cout << "8. Subscribe to Market Trades (WebSocket)\n";
            std::cout << "9. Exit\n";
            std::cout << "Enter your choice: ";
            
            int choice;
            std::cin >> choice;
            
            switch(choice) {
                case 1: {
                    // Place Spot Order
                    int instChoice, sideChoice, orderTypeChoice;
                    std::cout << "\nSelect Spot Instrument:\n";
                    std::cout << "1. BTC-USD\n";
                    std::cout << "2. ETH-USD\n";
                    std::cin >> instChoice;
                    SpotInstrument inst = (instChoice == 1) ? SpotInstrument::BTC_SPOT : SpotInstrument::ETH_SPOT;
                    
                    std::cout << "Select side (1 = BUY, 2 = SELL): ";
                    std::cin >> sideChoice;
                    OrderSide side = (sideChoice == 1) ? OrderSide::BUY : OrderSide::SELL;
                    
                    std::cout << "Select order type (1 = Market, 2 = Limit): ";
                    std::cin >> orderTypeChoice;
                    OrderType orderType = (orderTypeChoice == 1) ? OrderType::MARKET : OrderType::LIMIT;
                    
                    double amount, price = 0.0;
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    if (orderType == OrderType::LIMIT) {
                        std::cout << "Enter limit price: ";
                        std::cin >> price;
                    }
                    
                    SpotOrder order { inst, side, orderType, amount, price };
                    trade.placeSpotOrder(order);
                    break;
                }
                case 2: {
                    // Place Futures Order
                    int instChoice, sideChoice, orderTypeChoice;
                    std::cout << "\nSelect Futures Instrument:\n";
                    std::cout << "1. BTC-PERPETUAL\n";
                    std::cout << "2. ETH-PERPETUAL\n";
                    std::cout << "3. USDC-PERPETUAL\n";
                    std::cout << "4. USDT-PERPETUAL\n";
                    std::cout << "5. BTC-FUTURES\n";
                    std::cout << "6. ETH-FUTURES\n";
                    std::cin >> instChoice;
                    FuturesInstrument inst;
                    switch(instChoice) {
                        case 1: inst = FuturesInstrument::BTC_PERPETUAL; break;
                        case 2: inst = FuturesInstrument::ETH_PERPETUAL; break;
                        case 3: inst = FuturesInstrument::USDC_PERPETUAL; break;
                        case 4: inst = FuturesInstrument::USDT_PERPETUAL; break;
                        case 5: inst = FuturesInstrument::BTC_FUTURES; break;
                        case 6: inst = FuturesInstrument::ETH_FUTURES; break;
                        default: std::cout << "Invalid choice.\n"; continue;
                    }
                    
                    std::cout << "Select side (1 = BUY, 2 = SELL): ";
                    std::cin >> sideChoice;
                    OrderSide side = (sideChoice == 1) ? OrderSide::BUY : OrderSide::SELL;
                    
                    std::cout << "Select order type (1 = Market, 2 = Limit): ";
                    std::cin >> orderTypeChoice;
                    OrderType orderType = (orderTypeChoice == 1) ? OrderType::MARKET : OrderType::LIMIT;
                    
                    double amount, price = 0.0;
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    if (orderType == OrderType::LIMIT) {
                        std::cout << "Enter limit price: ";
                        std::cin >> price;
                    }
                    
                    std::string expiry;
                    std::cout << "Enter expiry date (YYYY-MM-DD): ";
                    std::cin >> expiry;
                    
                    FuturesOrder order { inst, side, orderType, amount, price, expiry };
                    trade.placeFuturesOrder(order);
                    break;
                }
                case 3: {
                    int instChoice, sideChoice, orderTypeChoice, optTypeChoice;
                    std::cout << "\nSelect Options Instrument:\n";
                    std::cout << "1. BTC-OPTIONS\n";
                    std::cout << "2. ETH-OPTIONS\n";
                    std::cin >> instChoice;
                    OptionsInstrument inst = (instChoice == 1) ? OptionsInstrument::BTC_OPTIONS : OptionsInstrument::ETH_OPTIONS;
                    
                    std::cout << "Select side (1 = BUY, 2 = SELL): ";
                    std::cin >> sideChoice;
                    OrderSide side = (sideChoice == 1) ? OrderSide::BUY : OrderSide::SELL;
                    
                    std::cout << "Select order type (1 = Market, 2 = Limit): ";
                    std::cin >> orderTypeChoice;
                    OrderType orderType = (orderTypeChoice == 1) ? OrderType::MARKET : OrderType::LIMIT;
                    
                    double amount, price = 0.0;
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    if (orderType == OrderType::LIMIT) {
                        std::cout << "Enter limit price: ";
                        std::cin >> price;
                    }
                    
                    std::string expiry;
                    std::cout << "Enter expiry date (YYYY-MM-DD): ";
                    std::cin >> expiry;
                    
                    double strike;
                    std::cout << "Enter strike price: ";
                    std::cin >> strike;
                    
                    std::cout << "Select option type (1 = CALL, 2 = PUT): ";
                    std::cin >> optTypeChoice;
                    OptionType optType = (optTypeChoice == 1) ? OptionType::CALL : OptionType::PUT;
                    
                    OptionsOrder order { inst, side, orderType, amount, price, expiry, strike, optType };
                    trade.placeOptionsOrder(order);
                    break;
                }
                case 4: {
                    // Cancel Order
                    std::string orderId;
                    std::cout << "\nEnter order ID to cancel: ";
                    std::cin >> orderId;
                    trade.cancelOrder(orderId);
                    break;
                }
                case 5: {
                    // Modify Order
                    std::string orderId;
                    double newAmount, newPrice;
                    std::cout << "\nEnter order ID to modify: ";
                    std::cin >> orderId;
                    std::cout << "Enter new amount: ";
                    std::cin >> newAmount;
                    std::cout << "Enter new price: ";
                    std::cin >> newPrice;
                    trade.modifyOrder(orderId, newAmount, newPrice);
                    break;
                }
                case 6: {
                    // View Current Positions
                    trade.viewCurrentPositions();
                    break;
                }
                case 7: {
                    // Subscribe to Order Book updates (WebSocket)
                    int n;
                    std::cout << "\nEnter number of instruments to subscribe for order book: ";
                    std::cin >> n;
                    std::vector<std::string> symbols;
                    for (int i = 0; i < n; ++i) {
                        std::string s;
                        std::cout << "Enter symbol " << i+1 << ":";
                        std::cin >> s;
                        symbols.push_back(s);
                    }
                    trade.subscribeToOrderBook(symbols);
                    break;
                }
                case 8: {
                    // Subscribe to Market Trades (WebSocket)
                    int n;
                    std::cout << "\nEnter number of instruments to subscribe for market trades:";
                    std::cin >> n;
                    std::vector<std::string> symbols;
                    for (int i = 0; i < n; ++i) {
                        std::string s;
                        std::cout << "Enter symbol " << i+1 << ": ";
                        std::cin >> s;
                        symbols.push_back(s);
                    }
                    trade.subscribeToMarketTrades(symbols);
                    break;
                }
                case 9:
                    running = false;
                    break;
                default:
                    std::cout << "Invalid option. Please try again.\n";
            }
        }
        
        std::cout << "Exiting trade engine.\n";
    } catch (const std::exception exp) {
        std::cerr << "Fatal error: " << exp.what() << std::endl;
        return 1;
    }

    
	return 0;
}
