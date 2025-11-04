#include "quote_api.h"
#include <random>
#include <sstream>

QuoteAPI::QuoteAPI() {
    initializeQuotes();
}

void QuoteAPI::initializeQuotes() {
    quotes_ = {
        {"시작이 반이다.", "한국 속담"},
        {"천 리 길도 한 걸음부터.", "한국 속담"},
        {"넘어지는 것은 두려운 일이 아니다. 다시 일어나지 않는 것이 두려운 일이다.", "넬슨 만델라"},
        {"꿈을 계속 간직하고 있으면 반드시 실현할 때가 온다.", "괴테"},
        {"노력하는 자는 즐기는 자를 이길 수 없다.", "공자"},
        {"오늘을 최선을 다해 살면 내일이 보인다.", "오프라 윈프리"},
        {"실패는 성공의 어머니다.", "한국 속담"},
        {"고생 끝에 낙이 온다.", "한국 속담"},
        {"백문이 불여일견.", "한국 속담"},
        {"뜻이 있는 곳에 길이 있다.", "링컨"},
        {"행동은 모든 성공의 기초이다.", "파블로 피카소"},
        {"배움에는 왕도가 없다.", "유클리드"},
        {"작은 기회로부터 위대한 업적이 시작된다.", "데모스테네스"},
        {"지금이 아니면 언제 하겠는가.", "탈무드"},
        {"인생은 가까이서 보면 비극이지만 멀리서 보면 희극이다.", "찰리 채플린"},
        {"할 수 있다고 믿는 사람은 결국 해낸다.", "베르길리우스"},
        {"최선을 다해서 실패한 것은 실패가 아니다.", "괴테"},
        {"인내하는 자에게 복이 있나니.", "성경"},
        {"어제의 나보다 오늘의 내가 나아지면 된다.", "이순신"},
        {"겨울이 오면 봄이 멀지 않으리.", "셸리"}
    };
}

ToolResult QuoteAPI::getRandomQuote(const json& arguments) {
    if (quotes_.empty()) {
        return {"text", "명언 데이터가 없습니다.", true};
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, quotes_.size() - 1);
    int randomIndex = dis(gen);

    const Quote& selectedQuote = quotes_[randomIndex];

    std::ostringstream result;
    result << "💭 오늘의 명언\n\n"
           << "\"" << selectedQuote.text << "\"\n\n"
           << "- " << selectedQuote.author;
    return {"text", result.str(), false};
}