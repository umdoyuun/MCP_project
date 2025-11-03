#pragma once
#include <string>
#include <vector>
#include "mcp_server.h"
using namespace std;

struct Quote{
    string text;
    string author;
};

class QuoteAPI{
private:
    vector<Quote> quotes_;
    void initializeQuotes();

public:
    QuoteAPI();
    ToolResult getRandomQuote(const json& arguments);
};