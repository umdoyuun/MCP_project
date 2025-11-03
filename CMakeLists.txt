cmake_minimum_required(VERSION 3.20)
project(mcp_server VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일 옵션
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    # MinGW 정적 링크 옵션 추가 - DLL 없이 실행 가능!
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

# vcpkg를 사용하는 경우 자동으로 찾음
find_package(nlohmann_json CONFIG QUIET)
find_package(CURL QUIET)

# nlohmann_json이 없으면 헤더 온리로 사용
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found via vcpkg, using include directory")
    include_directories(${CMAKE_SOURCE_DIR}/include)
endif()

# CURL이 없으면 경고
if(NOT CURL_FOUND)
    message(WARNING "CURL not found. HTTP features will be limited.")
endif()

# 소스 파일
set(SOURCES
    src/main.cpp
    src/mcp_server.cpp
    src/weather_api.cpp
    src/quote_api.cpp
)

# 실행 파일 생성
add_executable(mcp_server ${SOURCES})

# 라이브러리 링크
if(nlohmann_json_FOUND)
    target_link_libraries(mcp_server PRIVATE nlohmann_json::nlohmann_json)
endif()

if(CURL_FOUND)
    target_link_libraries(mcp_server PRIVATE CURL::libcurl)
    # Commented out to avoid Windows header conflicts with std::byte
    # target_compile_definitions(mcp_server PRIVATE USE_CURL)
    message(STATUS "CURL found but disabled due to Windows header conflicts")
endif()

# 헤더 파일 경로
target_include_directories(mcp_server PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Windows에서 추가 링크
if(WIN32)
    target_link_libraries(mcp_server PRIVATE ws2_32)
endif()

# 설치 규칙
install(TARGETS mcp_server DESTINATION bin)

# 버전 정보 출력
message(STATUS "Building MCP Server v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# 환경 변수 파일 경로 설정 (.env 파일)
set(ENV_FILE "${CMAKE_SOURCE_DIR}/.env")

# 파일이 존재하면 읽어서 환경변수로 설정
if(EXISTS ${ENV_FILE})
    file(READ ${ENV_FILE} ENV_CONTENTS)
    string(REPLACE "\n" ";" ENV_LINES "${ENV_CONTENTS}")

    foreach(line ${ENV_LINES})
        if(line MATCHES "^[A-Za-z_][A-Za-z0-9_]*=")
            string(REGEX MATCH "^[A-Za-z_][A-Za-z0-9_]*" VAR_NAME "${line}")
            string(REGEX REPLACE "^[A-Za-z_][A-Za-z0-9_]*=" "" VAR_VALUE "${line}")
            # 환경 변수 설정
            set(ENV{${VAR_NAME}} "${VAR_VALUE}")
            message(STATUS "Loaded env var: ${VAR_NAME}")
        endif()
    endforeach()
else()
    message(WARNING ".env file not found. API key might be missing.")
endif()