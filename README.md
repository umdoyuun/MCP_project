# C++ MCP Server

C++로 구현한 MCP(Model Context Protocol) Server입니다. Claude Desktop이나 다른 MCP 클라이언트와 연동하여 날씨 정보와 명언을 제공합니다.

## 특징

- **표준 준수**: MCP 프로토콜 공식 스펙 구현
- **경량화**: C++로 구현되어 빠른 실행 속도
- **확장 가능**: 새로운 Tool을 쉽게 추가 가능
- **크로스 플랫폼**: Windows, Linux, macOS 지원

## 제공 기능

### Tools

1. **getWeather**: 도시별 날씨 정보 조회
   - OpenWeatherMap API 연동
   - API 키가 없어도 샘플 데이터로 테스트 가능

2. **getRandomQuote**: 랜덤 명언 제공
   - 20개의 한국어 명언 내장
   - 매 호출마다 무작위 선택

## 요구사항

### 필수
- CMake 3.20 이상
- C++17 지원 컴파일러 (GCC, Clang, MSVC)
- nlohmann/json 라이브러리

### 선택
- CURL 라이브러리 (실제 날씨 API 사용 시)
- vcpkg (패키지 관리)

## 설치

### 1. 저장소 클론

```bash
git clone https://github.com/yourusername/cpp-mcp-server.git
cd cpp-mcp-server
```

### 2. 의존성 설치

#### vcpkg 사용 (권장)

```bash
# vcpkg 설치 (아직 없다면)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
# 또는
.\bootstrap-vcpkg.bat  # Windows

# 패키지 설치
./vcpkg install nlohmann-json
./vcpkg install curl  # 선택사항

# VCPKG_ROOT 환경변수 설정
export VCPKG_ROOT=/path/to/vcpkg  # Linux/macOS
# 또는
set VCPKG_ROOT=C:\path\to\vcpkg  # Windows
```

#### 수동 설치

nlohmann/json은 헤더 온리 라이브러리입니다. `include/` 디렉토리에 `json.hpp`를 다운로드하면 됩니다.

### 3. 빌드

#### Windows

```batch
.\build.bat
```

#### Linux/macOS

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

빌드가 완료되면 실행 파일이 생성됩니다:
- Windows: `build/Release/mcp_server.exe`
- Linux/macOS: `build/mcp_server`

## 사용법

### 1. 환경변수 설정 (선택사항)

실제 날씨 데이터를 사용하려면 OpenWeatherMap API 키가 필요합니다.

```bash
# Linux/macOS
export OPENWEATHER_API_KEY=your_api_key_here

# Windows (CMD)
set OPENWEATHER_API_KEY=your_api_key_here

# Windows (PowerShell)
$env:OPENWEATHER_API_KEY="your_api_key_here"
```

API 키는 [OpenWeatherMap](https://openweathermap.org/)에서 무료로 발급받을 수 있습니다.

### 2. 서버 실행

#### 직접 실행 (테스트용)

```bash
# Windows
.\build\Release\mcp_server.exe

# Linux/macOS
./build/mcp_server
```

#### Claude Desktop과 연동

Claude Desktop 설정 파일을 수정합니다.

**Windows**: `%APPDATA%\Claude\claude_desktop_config.json`

**macOS**: `~/Library/Application Support/Claude/claude_desktop_config.json`

**Linux**: `~/.config/Claude/claude_desktop_config.json`

```json
{
  "mcpServers": {
    "cpp-mcp-server": {
      "command": "C:/path/to/build/Release/mcp_server.exe",
      "env": {
        "OPENWEATHER_API_KEY": "your_api_key_here"
      }
    }
  }
}
```

설정 후 Claude Desktop을 재시작하면 서버가 자동으로 연결됩니다.

### 3. 테스트

Python 테스트 스크립트를 제공합니다.

```bash
python test.py
```

## 프로젝트 구조

```
cpp-mcp-server/
├── src/
│   ├── main.cpp              # 메인 진입점
│   ├── mcp_server.cpp        # MCP 서버 구현
│   ├── mcp_server.h          # MCP 서버 헤더
│   ├── weather_api.cpp       # 날씨 API 구현
│   ├── weather_api.h         # 날씨 API 헤더
│   ├── quote_api.cpp         # 명언 API 구현
│   └── quote_api.h           # 명언 API 헤더
├── include/
│   └── json.hpp              # nlohmann/json (선택)
├── CMakeLists.txt            # CMake 빌드 설정
├── build.bat                 # Windows 빌드 스크립트
├── test.py                   # 테스트 스크립트
└── README.md
```

## 새로운 Tool 추가하기

1. API 클래스 작성

```cpp
// my_api.h
#pragma once
#include "mcp_server.h"

class MyAPI {
public:
    ToolResult myFunction(const json& arguments);
};
```

2. Tool 등록

```cpp
// main.cpp
MyAPI myApi;

Tool myTool{
    "myFunction",
    "My function description",
    {
        {"type", "object"},
        {"properties", {
            {"param1", {
                {"type", "string"},
                {"description", "Parameter description"}
            }}
        }},
        {"required", json::array({"param1"})}
    }
};

server.registerTool(myTool, 
    [&myApi](const json& args) {
        return myApi.myFunction(args);
    }
);
```

## MCP 프로토콜 구조

### 메시지 형식

```
Content-Length: <byte_count>\r\n
\r\n
<JSON-RPC message>
```

### JSON-RPC 메시지

```json
{
  "jsonrpc": "2.0",
  "id": "unique_id",
  "method": "method_name",
  "params": {}
}
```

### 지원 메서드

- `initialize`: 서버 초기화
- `tools/list`: Tool 목록 조회
- `tools/call`: Tool 실행

## 기술 스택

- **언어**: C++17
- **빌드 시스템**: CMake
- **JSON 파싱**: nlohmann/json
- **HTTP 통신**: libcurl (선택)
- **프로토콜**: JSON-RPC 2.0

## 문제 해결

### CURL을 찾을 수 없음

```
✗ CURL not found - Weather API will return sample data only
```

CURL 라이브러리가 설치되지 않았습니다. vcpkg로 설치하거나, API 키 없이 샘플 데이터로 테스트할 수 있습니다.

```bash
vcpkg install curl
```

### nlohmann/json을 찾을 수 없음

```
Could not find nlohmann_json
```

vcpkg로 설치하거나, `include/` 디렉토리에 `json.hpp`를 직접 다운로드하세요.

```bash
vcpkg install nlohmann-json
```

### 빌드 오류 (Windows)

MSVC 컴파일러가 설치되어 있는지 확인하세요. Visual Studio 2019 이상이 필요합니다.

### 실행 오류 (Linux)

```bash
./mcp_server: error while loading shared libraries: libcurl.so.4
```

CURL 라이브러리를 설치하세요.

```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# CentOS/RHEL
sudo yum install libcurl-devel
```

## 라이선스

MIT License

## 참고 자료

- [MCP 공식 문서](https://modelcontextprotocol.io/)
- [JSON-RPC 2.0 스펙](https://www.jsonrpc.org/specification)
- [nlohmann/json](https://github.com/nlohmann/json)
- [OpenWeatherMap API](https://openweathermap.org/api)

## 기여

버그 리포트나 기능 제안은 Issue를 통해 알려주세요. Pull Request도 환영합니다!

## 작성자

- GitHub: [@yourusername](https://github.com/yourusername)
- Blog: https://doyun98.tistory.com/