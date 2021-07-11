#pragma once

#include "SqLog.h"
#include "../Squinky.hpp"

class GMRScreenHolder : public OpaqueWidget {
public:
    GMRScreenHolder(const Vec &pos, const Vec &size);
    ~GMRScreenHolder();
    void draw(const DrawArgs &args) override;

private:

    /**
     * these are the screens we are managing.
     * at any one time one of them will be a child of us
     */
    std::vector<Widget*> screens; 
    void onNewTab(int index);
    int currentTab=0; 

    void sizeChild(Widget*);         
};
