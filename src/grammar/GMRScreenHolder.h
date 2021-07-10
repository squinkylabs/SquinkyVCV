#pragma once

#include "FakeScreen.h"

class GMRScreenHolder : public OpaqueWidget {
public:
    GMRScreenHolder(const Vec &pos, const Vec &size);
    void draw(const DrawArgs &args) override;

private:
    FakeScreen *child1 = nullptr;
    FakeScreen *child2 = nullptr;
    float childPos = .25;
};
