#pragma once
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <vector>
#include "json.hpp"
using json = nlohmann::json;

struct Tool{
    std::string name;
    std::string description;
    json inputSchema;
};

struct ToolResult{
    std::string type;
    std::string text;
    bool isError;
};

using ToolHandler = std::function<ToolResult(const json&)>;

class MCPServer{
private:
    std::string serverName_;
    std::vector<Tool> tools_;
    std::map<std::string, ToolHandler> toolHandlers_;

    void handleMessage(const json& request);

    json createResponse(const json& id, const json& result);
    json createErrorResponse(const json& id, int code, const std::string& errorMessage);

    void handleInitialize(const json& id, const json& params);
    void handleListTools(const json& id);
    void handleCallTool(const json& id, const json& params);

    void sendMessage(const json& message);

    std::string readMessage();
public:
    MCPServer(const std::string& serverName);

    void registerTool(const Tool& tool, ToolHandler handler);

    void run();
};