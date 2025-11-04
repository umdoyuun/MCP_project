#pragma once
#include <string>
#include <vector>
#include "mcp_server.h"

struct Quote{
    std::string text;
    std::string author;
};

class QuoteAPI{
private:
    std::vector<Quote> quotes_;
    void initializeQuotes();

public:
    QuoteAPI();
    ToolResult getRandomQuote(const json& arguments);
};