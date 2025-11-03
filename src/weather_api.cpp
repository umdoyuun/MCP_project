#include "weather_api.h"
#include <sstream>
#include <iomanip>

#ifdef USE_CURL
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
#endif

WeatherAPI::WeatherAPI(const string& apiKey)
    : apiKey_(apiKey) {
}

ToolResult WeatherAPI::getWeather(const json& arguments) {
    if (!arguments.contains("city") || !arguments["city"].is_string()) {
        return {"text", "Invalid arguments: 'city' is required.", true};
    }

    string city = arguments.value("city", "Seoul");
    
    // API 키가 없으면 샘플 데이터 반환
    if (apiKey_.empty()) {
        ostringstream result;
        result << "⚠️ API 키가 설정되지 않았습니다.\n\n"
               << "샘플 데이터 - " << city << "의 날씨:\n"
               << "🌡️ 온도: 22°C\n"
               << "☁️ 날씨: 맑음\n"
               << "💧 습도: 60%\n"
               << "💨 풍속: 3.5 m/s\n\n"
               << "실제 데이터를 받으려면 OpenWeatherMap API 키를 설정하세요.";
        return {"text", result.str(), false};
    }
    
    string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey_ + "&units=metric&lang=kr";

    string response = httpGet(url);
    if (response.empty()) {
        return {"text", "날씨 데이터를 가져오는데 실패했습니다.", true};
    }

    try {
        json weatherData = json::parse(response);
        if (weatherData.contains("cod") && weatherData["cod"] != 200) {
            string message = weatherData.value("message", "Unknown error");
            return {"text", "오류: " + message, true};
        }

        ostringstream result;
        result << "🌤️ " << city << "의 현재 날씨:\n\n";
        result << "🌡️ 온도: " << fixed << setprecision(1) << weatherData["main"]["temp"] << " °C\n";
        result << "🤔 체감온도: " << weatherData["main"]["feels_like"] << " °C\n";
        result << "💧 습도: " << weatherData["main"]["humidity"] << " %\n";
        result << "☁️ 날씨: " << weatherData["weather"][0]["description"].get<string>() << "\n";

        return {"text", result.str(), false};
    } catch (const std::exception& e) {
        return {"text", string("날씨 데이터 파싱 오류: ") + e.what(), true};
    }
}

string WeatherAPI::httpGet(const string& url) {
#ifdef USE_CURL
    // CURL이 활성화된 경우
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
#else
    // CURL이 비활성화된 경우 빈 문자열 반환
    return "";
#endif
}