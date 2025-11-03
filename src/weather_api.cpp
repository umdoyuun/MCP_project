#include "weather_api.h"
#include <sstream>
#include <iomanip>
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WeatherAPI::WeatherAPI(const string& apiKey)
    : apiKey_(apiKey) {
}

ToolResult WeatherAPI::getWeather(const json& arguments) {
    if (!arguments.contains("city") || !arguments["city"].is_string()) {
        return {"text", "Invalid arguments: 'city' is required.", true};
    }

    string city = arguments.value("city", "Seoul");
    string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey_ + "&units=metric";

    string response = httpGet(url);
    if (response.empty()) {
        return {"text", "Failed to retrieve weather data.", true};
    }

    try {
        json weatherData = json::parse(response);
        if (weatherData.contains("cod") && weatherData["cod"] != 200) {
            string message = weatherData.value("message", "Unknown error");
            return {"text", "Error: " + message, true};
        }

        ostringstream result;
        result << "🌤️ " << city << "의 현재 날씨:\n\n";
        result << "온도: " << fixed << setprecision(1) << weatherData["main"]["temp"] << " °C\n";
        result << "체감온도: " << weatherData["main"]["feels_like"] << " °C\n";
        result << "습도: " << weatherData["main"]["humidity"] << " %\n";
        result << "날씨: " << weatherData["weather"][0]["description"].get<string>() << "\n";

        return {"text", result.str(), false};
    } catch (const std::exception& e) {
        return {"text", string("Error parsing weather data: ") + e.what(), true};
    }
}

string WeatherAPI::httpGet(const string& url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw runtime_error(curl_easy_strerror(res));
        }
    }
    return readBuffer;
}