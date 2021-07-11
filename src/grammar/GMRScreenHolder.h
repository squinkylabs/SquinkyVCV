#pragma once

#include "FakeScreen.h"

class GMRScreenHolder : public TransparentWidget {
public:
    GMRScreenHolder(const Vec &pos, const Vec &size);
    void draw(const DrawArgs &args) override;

private:

    /**
     * these are the screens we are managing.
     * at any one time one of them will be a child of us
     */
    std::vector<Widget*> screens;           
};
