#pragma once

#include "../Squinky.hpp"
using FontPtr = std::shared_ptr<Font>;

class TextUtils {
public:
    enum class WhichFont {
        regular,
        bold
    };
    static FontPtr loadFont(WhichFont);
};
