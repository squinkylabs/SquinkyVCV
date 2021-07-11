#pragma once

#include "SqLog.h"
#include "../Squinky.hpp"

class ProductionRuleEditor : public OpaqueWidget {
public:
    ~ProductionRuleEditor() { SQINFO("dtor of ProductionRuleEditor"); }
    void draw(const DrawArgs &args) override;
};