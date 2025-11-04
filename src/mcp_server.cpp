#include "mcp_server.h"
#include <iostream>
#include <sstream>
#include <cstring>

MCPServer::MCPServer(const std::string& serverName)
 : serverName_(serverName) {
}

void MCPServer::registerTool(const Tool& tool, ToolHandler handler) {
    tools_.push_back(tool);
    toolHandlers_[tool.name] = handler;
}

void MCPServer::run() {
    while (true) {
        try{
            std::string messageStr = readMessage();
            if (messageStr.empty()) {
                break;
            }

            json request = json::parse(messageStr, nullptr, false);
            if (request.is_discarded()) {
                std::cerr << "Failed to parse JSON message: " << messageStr << std::endl;
                continue;
            }
            handleMessage(request);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            json error = createErrorResponse(json(nullptr), -32700, "Parse error");
            sendMessage(error);
        }        
        
    }
}

void MCPServer::handleMessage(const json& request) {
    if(!request.contains("jsonrpc") || request["jsonrpc"] != "2.0"){
        sendMessage(createErrorResponse(request.value("id", json(nullptr)), -32600, "Invalid Request"));
        return;
    }

    std::string method = request.value("method", "");
    json id = request.value("id", json(nullptr));
    json params = request.value("params", json::object());

    if(method == "initialize"){
        handleInitialize(id, params);
    } else if(method == "tools/list"){
        handleListTools(id);
    } else if(method == "tools/call"){
        handleCallTool(id, params);
    } else {
        sendMessage(createErrorResponse(id, -32601, "Method not found"));
    }
}

void MCPServer::handleInitialize(const json& id, const json& params) {
    json result = {
        {"protocolVersion", "2.0"},
        {"serverInfo",{
            {"name", serverName_},
            {"version", "1.0.0"}
        }},
        {"capabilities",{
            {"tools", json::object()}
        }}
    };

    sendMessage(createResponse(id, result));
}

void MCPServer::handleListTools(const json& id) {
    json toolList = json::array();

    for (const auto& tool : tools_) {
        toolList.push_back({
            {"name", tool.name},
            {"description", tool.description},
            {"inputSchema", tool.inputSchema}
        });
    }

    json result = {{"tools", toolList}};
    sendMessage(createResponse(id, result));
}

void MCPServer::handleCallTool(const json& id, const json& params) {
    if (!params.contains("name") || !params["name"].is_string()) {
        sendMessage(createErrorResponse(id, -32602, "Invalid params: 'name' is required"));
        return;
    }

    std::string toolName = params["name"];
    json arguments = params.value("arguments", json::object());

    auto it = toolHandlers_.find(toolName);
    if (it == toolHandlers_.end()) {
        sendMessage(createErrorResponse(id, -32602, "Tool not found: " + toolName));
        return;
    }

    try{
        ToolResult result = it->second(arguments);

        json response = {
            {"content", json::array({
                {
                    {"type", result.type},
                    {"text", result.text}
                }
            })},
            {"isError", result.isError}
        };

        sendMessage(createResponse(id, response));
    }
    catch(const std::exception& e){
        sendMessage(createErrorResponse(id, -32603, std::string("Internal error: ") + e.what()));
    }
}

json MCPServer::createResponse(const json& id, const json& result) {
    return {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };
}

json MCPServer::createErrorResponse(const json& id, int code, const std::string& errorMessage) {
    return {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {
            {"code", code},
            {"message", errorMessage}
        }}
    };
}

void MCPServer::sendMessage(const json& message) {
    std::string messageStr = message.dump();

    std::cout << "Content-Length: " << messageStr.size() << "\r\n\r\n";
    std::cout << messageStr << std::flush;
    
    std::cerr << "[SEND] " << messageStr.substr(0, 100) 
         << (messageStr.size() > 100 ? "..." : "") << std::endl;
}

std::string MCPServer::readMessage() {
    std::string line;
    size_t contentLength = 0;
    
    while (getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) {
            break;
        }
        
        if (line.find("Content-Length:") == 0) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string lengthStr = line.substr(colonPos + 1);
                size_t start = lengthStr.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    lengthStr = lengthStr.substr(start);
                }
                contentLength = stoull(lengthStr);
            }
        }
    }
    
    if (contentLength == 0) {
        return "";
    }
    
    std::string content;
    content.resize(contentLength);
    std::cin.read(&content[0], contentLength);
    
    if (std::cin.gcount() != static_cast<std::streamsize>(contentLength)) {
        std::cerr << "Failed to read full message body" << std::endl;
        return "";
    }
    
    std::cerr << "[RECV] " << content.substr(0, 100) 
         << (content.size() > 100 ? "..." : "") << std::endl;
    
    return content;
}