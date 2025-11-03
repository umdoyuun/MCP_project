#pragma once
#include <string>
#include <vector>
#include "mcp_server.h"
using namespace std;

struct Quote{
    string author;
    string text;
};

class QuoteAPI{
private:
    vector<Quote> quotes_;
    void initalizeQuotes();
    
public:
    QuoteAPI();
    ToolResult getRandomQuote(const json& arguments);
}