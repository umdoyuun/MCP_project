#include "mcp_server.h"
#include "weather_api.h"
#include "quote_api.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

int main(){
    // Windows에서 stdin/stdout을 바이너리 모드로 설정
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    try{
        MCPServer server("MyMCPServer");

        const char* weatherApiKey = std::getenv("OPENWEATHER_API_KEY");
        
        WeatherAPI weatherApi(weatherApiKey ? weatherApiKey : "");
        QuoteAPI quoteApi;

        Tool weatherTool{
            "getWeather",
            "특정 도시의 현재 날씨 정보를 제공합니다.",
            {
                {"type", "object"},
                {"properties", {
                    {"city", {
                        {"type", "string"},
                        {"description", "날씨 정보를 조회할 도시 이름(예: Seoul, London 등)"}
                    }}
                }},
                {"required", json::array({"city"})}
            }
        };

        server.registerTool(weatherTool, 
            [&weatherApi](const json& args) {
                return weatherApi.getWeather(args);
            }
        );

        Tool quoteTool{
            "getRandomQuote",
            "무작위 명언을 제공합니다.",
            {
                {"type", "object"},
                {"properties", json::object()}
            }
        };

        server.registerTool(quoteTool,
            [&quoteApi](const json& args) {
                return quoteApi.getRandomQuote(args);
            }
        );

        cerr << "MCP Server is running..." << endl;
        
        server.run();

        return 0;
    }
    catch (const std::exception& e){
        cerr << "Fatal Error: " << e.what() << endl;
        return 1;
    }
}