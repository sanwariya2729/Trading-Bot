#ifndef AUTHORISATION_H
#define AUTHORISATION_H

#include <string>
#include <mutex>
#include <ctime>
#include "Logger.h"



class Authorization {
public:
    Authorization(const std::string& clientId, const std::string& clientSecret);
    // Returns the current valid access token (refreshes if expired)
    std::string getAccessToken();
    std::string getClientId(){
    	return clientId;
    }
    std::string getClientSecret(){
    	return clientSecret;
    }


private:
    std::string clientId;
    std::string clientSecret;
    std::string accessToken;
    std::string refreshToken;
    time_t expiryTime; // Unix timestamp when token expires
    std::mutex tokenMutex;
 
    // Makes a POST request to Deribit and returns the response as a string.
    std::string httpPost(const std::string& url, const std::string& jsonPayload);
    
    
    // Perform initial authentication to get tokens.
    bool performAuthentication();
    
    // Perform a token refresh.
    bool performTokenRefresh();
    
    // Update token details.
    void updateToken(const std::string& newToken, const std::string& newRefreshToken, int expiresIn);
};

#endif 

