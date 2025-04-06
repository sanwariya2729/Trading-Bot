#include "Authorisation.h"
#include <iostream>
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback for libcurl to write response data into a std::string.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* response = reinterpret_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    response->append(reinterpret_cast<char*>(contents), totalSize);
    return totalSize;
}

Authorization::Authorization(const std::string& clientId, const std::string& clientSecret)
    : clientId(clientId), clientSecret(clientSecret), expiryTime(0) 
{
	initLogger();
    std::lock_guard<std::mutex> lock(tokenMutex);
    systemLogger->info("[Auth] performing authentication");
    if (!performAuthentication()) {
        systemLogger->error("[Auth] Initial authentication failed.");
    }
}

std::string Authorization::httpPost(const std::string& url, const std::string& jsonPayload) {
    CURL* curl = curl_easy_init();
    std::string response;
    if(curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            systemLogger->error("[Auth] curl_easy_perform() failed: {}", curl_easy_strerror(res));
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            systemLogger->info("[Auth] HTTP response code: {}", http_code);

            // Handle non-200 status codes explicitly
            if (http_code != 200) {
                systemLogger->error("[Auth] HTTP error: {} | Response: {}", http_code, response);
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    
    return response;
}


bool Authorization::performAuthentication() {
    std::string url = "https://test.deribit.com/api/v2/public/auth";
    json payload = {
    {"jsonrpc", "2.0"},
    {"id", 1},
    {"method", "public/auth"},
    {"params", {
        {"grant_type", "client_credentials"},
        {"client_id", clientId},
        {"client_secret", clientSecret},
        {"scope", "read write"}
    }}
};

    std::string response = httpPost(url, payload.dump());

    try {
        auto j = json::parse(response);
        if (j.contains("result")) {
            auto res = j["result"];
            // Proactively refresh token 60 seconds before expiry.
            int expiresIn = res["expires_in"].get<int>();
            updateToken(res["access_token"].get<std::string>(),
                        res["refresh_token"].get<std::string>(),
                        expiresIn);
            return true;
        } else {
            systemLogger->error("[Auth] Authentication response error: {} ",response);
        }
    } catch (json::exception& e) {
        systemLogger->error("[Auth] JSON parsing error: {}", e.what() , " Response: {}",response);
    }
    return false;
}

bool Authorization::performTokenRefresh() {

    std::string url = "https://test.deribit.com/api/v2/public/auth";
    // Build the JSON payload for refresh_token grant.
    json payload = {
  	{"jsonrpc", "2.0"},
  	{"id", 1},
  	{"method", "public/auth"},
  	{"params", {
    	{"grant_type", "refresh_token"},
    	{"refresh_token",refreshToken }
  }}
};

    std::string response = httpPost(url, payload.dump());
    try {
        auto j = json::parse(response);
        if (j.contains("result")) {
            auto res = j["result"];
            updateToken(res["access_token"].get<std::string>(),
                        res["refresh_token"].get<std::string>(),
                        res["expires_in"].get<int>());
            return true;
        } else {
            systemLogger->error("[Auth] Token refresh response error: {}",response);
        }
    } catch (json::exception& e) {
        systemLogger->error("[Auth] JSON parsing error during refresh: {}", e.what() );
    }
    return false;
}

void Authorization::updateToken(const std::string& newToken, const std::string& newRefreshToken, int expiresIn) {
    accessToken = newToken;
    refreshToken = newRefreshToken;
    expiryTime = std::time(nullptr) + expiresIn;
}

std::string Authorization::getAccessToken() {
    std::lock_guard<std::mutex> lock(tokenMutex);
    time_t currentTime = std::time(nullptr);

    // Refresh token proactively if it expires in less than 60 seconds.
    if (currentTime >= (expiryTime - 60)) {
        systemLogger->info("[Auth] Token nearing expiry or expired. Refreshing token...");
        // Retry mechanism: try up to 3 times if refresh fails.
        const int maxRetries = 3;
        for (int attempt = 1; attempt <= maxRetries; ++attempt) {
            if (performTokenRefresh()) {
                break; 
            } else {
                systemLogger->info("[Auth] Token refresh attempt {}" ,attempt, " failed. Retrying in 5 seconds..." );
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }
    return accessToken;
}




