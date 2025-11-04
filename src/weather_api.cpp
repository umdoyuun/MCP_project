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

WeatherAPI::WeatherAPI(const std::string& apiKey)
    : apiKey_(apiKey) {
}

ToolResult WeatherAPI::getWeather(const json& arguments) {
    if (!arguments.contains("city") || !arguments["city"].is_string()) {
        return {"text", "❌ 잘못된 요청: 'city' 매개변수가 필요합니다.", true};
    }

    std::string city = arguments["city"];
    
    // ✅ 개선: API 키 상태에 따른 명확한 메시지
    if (apiKey_.empty()) {
        std::ostringstream result;
        result << "⚠️ OpenWeatherMap API 키가 설정되지 않았습니다.\n\n"
               << "📍 샘플 데이터 - " << city << "의 날씨:\n\n"
               << "🌡️ 온도: 22°C\n"
               << "🤔 체감온도: 20°C\n"
               << "💧 습도: 60%\n"
               << "☁️ 날씨: 맑음\n"
               << "💨 풍속: 3.5 m/s\n\n"
               << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
               << "💡 실제 날씨 데이터를 받으려면:\n"
               << "1. https://openweathermap.org/ 에서 무료 API 키 발급\n"
               << "2. 환경변수 설정: OPENWEATHER_API_KEY=your_key\n"
               << "3. 서버 재시작";
        return {"text", result.str(), false};
    }
    
#ifdef USE_CURL
    // ✅ CURL이 활성화된 경우 실제 API 호출
    std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" 
                 + city + "&appid=" + apiKey_ + "&units=metric&lang=kr";

    std::string response = httpGet(url);
    if (response.empty()) {
        return {"text", "❌ 날씨 데이터를 가져오는데 실패했습니다.\n네트워크 연결을 확인해주세요.", true};
    }

    try {
        json weatherData = json::parse(response);
        
        // API 에러 체크
        if (weatherData.contains("cod")) {
            // cod가 문자열일 수도, 숫자일 수도 있음
            int code = 0;
            if (weatherData["cod"].is_string()) {
                code = stoi(weatherData["cod"].get<std::string>());
            } else if (weatherData["cod"].is_number()) {
                code = weatherData["cod"].get<int>();
            }
            
            if (code != 200) {
                std::string message = weatherData.value("message", "Unknown error");
                return {"text", "❌ API 오류: " + message, true};
            }
        }

        // 성공적으로 데이터 파싱
        std::ostringstream result;
        result << "🌤️ " << city << "의 현재 날씨:\n\n";
        result << "🌡️ 온도: " << std::fixed << std::setprecision(1) 
               << weatherData["main"]["temp"].get<double>() << " °C\n";
        result << "🤔 체감온도: " 
               << weatherData["main"]["feels_like"].get<double>() << " °C\n";
        result << "💧 습도: " << weatherData["main"]["humidity"].get<int>() << " %\n";
        
        if (weatherData.contains("weather") && !weatherData["weather"].empty()) {
            std::string description = weatherData["weather"][0]["description"].get<std::string>();
            result << "☁️ 날씨: " << description << "\n";
        }
        
        if (weatherData.contains("wind") && weatherData["wind"].contains("speed")) {
            result << "💨 풍속: " << weatherData["wind"]["speed"].get<double>() << " m/s\n";
        }

        return {"text", result.str(), false};
    } catch (const std::exception& e) {
        return {"text", std::string("❌ 날씨 데이터 파싱 오류: ") + e.what(), true};
    }
#else
    // ✅ CURL이 비활성화된 경우
    std::ostringstream result;
    result << "⚠️ HTTP 기능이 비활성화되어 있습니다.\n\n"
           << "📍 샘플 데이터 - " << city << "의 날씨:\n\n"
           << "🌡️ 온도: 22°C\n"
           << "🤔 체감온도: 20°C\n"
           << "💧 습도: 60%\n"
           << "☁️ 날씨: 맑음\n"
           << "💨 풍속: 3.5 m/s\n\n"
           << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
           << "💡 실제 날씨 API를 사용하려면:\n"
           << "1. CURL 라이브러리 설치\n"
           << "2. CMakeLists.txt에서 USE_CURL 활성화\n"
           << "3. 프로젝트 재빌드";
    return {"text", result.str(), false};
#endif
}

std::string WeatherAPI::httpGet(const std::string& url) {
#ifdef USE_CURL
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // ✅ 타임아웃 설정
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // ✅ 리다이렉트 따라가기
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(res));
        }
    }
    return readBuffer;
#else
    // CURL이 비활성화된 경우 빈 문자열 반환
    (void)url; // ✅ 경고 제거
    return "";
#endif
}