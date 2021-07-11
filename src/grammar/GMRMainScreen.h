#pragma once

#include "SqLog.h"
#include "../Squinky.hpp"

class GMRMainScreen   : public OpaqueWidget {
public:
    ~GMRMainScreen() { SQINFO("dtor GMRMainScreen"); }
    void draw(const DrawArgs &args) override;
};
