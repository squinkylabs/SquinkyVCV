#pragma once

#include <functional>
#include <vector>

namespace rack {
    namespace math {
        struct Vec;    
    }
}

class InputScreenManager
{
public:
    enum class Screens { test };
    using Callback = std::function<void(std::vector<float>)>;

    InputScreenManager(::rack::math::Vec size);
    /**
     * If you call it while a screen is up the call will be ignored.
     */
    void show (::rack::widget::Widget* parent, Screens, Callback);

    /**
     * If you destroy the manager while a screen is up,
     * it's ok. Everything will be cleaned up
     */
    ~InputScreenManager();

     ::rack::widget::Widget* parentWidget = nullptr;
private:
    const ::rack::math::Vec size;

};