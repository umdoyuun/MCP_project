#pragma once
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

struct Tool{
    string name;
    string description;
    json inputSchema;
};

struct ToolResult{
    string type;
    string text;
    bool isError;
};

using ToolHandler = std::function<ToolResult(const json&)>;

class MCPServer{
private:
    string serverName_;
    vector<Tool> tools_;
    map<string, ToolHandler> toolHandlers_;

    void handleMessage(const json& request);

    json createResponse(const string& id, const json& result);
    json createErrorResponse(const string& id, int code, const string& errorMessage);

    void haddleInitialize(const json& id, const json& result);
    void handleListTools(const json& id);
    void handleCallTool(const json& id, const json& params);

    void sendmessage(const json& message);

    string readMessage();
public:
    MCPServer(const string& serverName);

    void registerTool(const Tool& tool, ToolHandler handler);

    void run();
};