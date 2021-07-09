#pragma once

class ScreenHolder : public OpaqueWidget {
public:
    ScreenHolder(const Vec &pos, const Vec &size);
    void draw(const DrawArgs &args) override;
};

inline ScreenHolder::ScreenHolder(const Vec &pos, const Vec &size) {
    this->box.pos = pos;
    this->box.size = size;
}

inline void
ScreenHolder::draw(const DrawArgs &args) {
}