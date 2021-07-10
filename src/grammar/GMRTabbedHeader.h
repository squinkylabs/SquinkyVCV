#pragma once

#include "../Squinky.hpp"

using FontPtr = std::shared_ptr<Font>;

class GMRTabbedHeader  : public OpaqueWidget {
public:
    GMRTabbedHeader();
    GMRTabbedHeader(const GMRTabbedHeader&) = delete;
    
    void draw(const DrawArgs &args) override;
private:
    FontPtr regFont;
    FontPtr boldFont;

};