#include "mcp_server.h"
#include <iostream>
#include <sstream>
#include <cstring>
using namespace std;

MCPServer::MCPServer(const string& serverName)
 : serverName_(serverName) {
}

void MCPServer::registerTool(const Tool& tool, ToolHandler handler) {
    tools_.push_back(tool);
    toolHandlers_[tool.name] = handler;
}

void MCPServer::run() {
    while (true) {
        try{
            string messageStr = readMessage();
            if (messageStr.empty()) {
                break;
            }

            json request = json::parse(messageStr, nullptr, false);
            if (request.is_discarded()) {
                cerr << "Failed to parse JSON message: " << messageStr << endl;
                continue;
            }
            handleMessage(request);
        }
        catch (const std::exception& e) {
            cerr << "Exception: " << e.what() << endl;
            json error = createErrorResponse(nullptr, -32700, "Parse error");
            sendMessage(error);
        }        
        
    }
}

void MCPServer::handleMessage(const json& request) {
    if(!request.contains("jsonrpc") || request["jsonrpc"] != "2.0"){
        sendMessage(createErrorResponse(request.value("id", nullptr), -32600, "Invalid Request"));
        return;
    }

    string method = request.value("method", "");
    json id = request.value("id", nullptr);
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
    string toolName = params.value("name", "");
    json arguments = params.value("arguments", json::object());

    auto it = toolHandlers_.find(toolName);
    if (it == toolHandlers_.end()) {
        sendMessage(createErrorResponse(id, -32602, "Tool not found"));
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
        sendMessage(createErrorResponse(id, -32603, string("Internal error: ") + e.what()));
    }
}

json MCPServer::createResponse(const string& id, const json& result) {
    return {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };
}

json MCPServer::createErrorResponse(const string& id, int code, const string& errorMessage) {
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
    string messageStr = message.dump();

    // Prepare header
    stringstream header;
    header << "Content-Length: " << messageStr.size() << "\r\n\r\n";

    // Send header and message
    cout << header.str();
    cout << messageStr;
    cout.flush();
}

string MCPServer::readMessage() {
    string line;
    if(getline(cin, line)){
        return line;
    }
    return "";
}