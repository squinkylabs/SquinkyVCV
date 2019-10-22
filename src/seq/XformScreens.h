#pragma once

#include "InputScreen.h"
class XformInvert : public InputScreen
{
public:
    XformInvert(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        std::function<void()> _dismisser);
};