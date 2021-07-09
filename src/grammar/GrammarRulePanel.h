#pragma once

#include <sstream>
#include "../Squinky.hpp"
#include "WidgetComposite.h"


#include "widget/Widget.hpp"

#include <memory>

class StochasticGrammar;
class GMRModule;

using StochasticGrammarPtr = std::shared_ptr<StochasticGrammar>;

class GrammarRulePanel : public OpaqueWidget {
public:
    GrammarRulePanel(const Vec &pos, const Vec &size, StochasticGrammarPtr grammar, Module* module);
    void draw(const DrawArgs &args) override;
private:
    Module* module;
};
