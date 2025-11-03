#pragma once
#include <string>
#include "mcp_server.h"
using namespace std;

class WeatherAPI{
private:
    string apiKey_;
    
    string httpGet(const string& url);

public:
    WeatherAPI(const string& apiKey = "");

    ToolResult getWeather(const json& arguments);
};