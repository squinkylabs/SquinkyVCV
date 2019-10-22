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
    enum class Screens { Invert };
    using Callback = std::function<void(std::vector<float>)>;

    InputScreenManager(::rack::math::Vec size);
    /**
     * If you call it while a screen is up the call will be ignored.
     */
    // TODO: get rid of callback
    void show (::rack::widget::Widget* parent, Screens, MidiSequencerPtr, Callback);

    /**
     * If you destroy the manager while a screen is up,
     * it's ok. Everything will be cleaned up
     */
    ~InputScreenManager();

     ::rack::widget::Widget* parentWidget = nullptr;
private:
    const ::rack::math::Vec size;
    InputScreenPtr screen; 
    ::rack::widget::Widget* parent = nullptr;

    void dismiss();

    template <class T>
    std::shared_ptr<T> make(
        const ::rack::math::Vec& size,  
        MidiSequencerPtr seq, 
        std::function<void()> dismisser);

};