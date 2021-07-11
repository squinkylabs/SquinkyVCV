#pragma once

#include "../Squinky.hpp"
#include "SqLog.h"

class StochasticGrammar;
using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;

class GMRMainScreen : public OpaqueWidget {
public:
    GMRMainScreen(StochasticGrammarPtr);
    ~GMRMainScreen() { SQINFO("dtor GMRMainScreen"); }

    void draw(const DrawArgs &args) override;

private:
    StochasticGrammarPtr grammar;
};
