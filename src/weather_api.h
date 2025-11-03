#pragma once
#include <string>
#include "mcp_server.h"

class WeatherAPI{
private:
    std::string apiKey_;
    
    std::string httpGet(const std::string& url);

public:
    WeatherAPI(const std::string& apiKey = "");

    ToolResult getWeather(const json& arguments);
};