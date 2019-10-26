#pragma once

#include <functional>
#include <vector>

class InputScreen;
namespace rack {
    namespace math {
        struct Vec;    
    }
}

using InputScreenPtr = std::shared_ptr<InputScreen>;

class InputScreenManager
{
public:
    enum class Screens { Invert, Transpose };
    using Callback = std::function<void()>;

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

     
    static std::string xformName(Screens);
private:
    ::rack::widget::Widget* parentWidget = nullptr;
    const ::rack::math::Vec size;
    InputScreenPtr screen; 
    ::rack::widget::Widget* parent = nullptr;
    Callback callback = nullptr;

    void dismiss(bool bOK);

    template <class T>
    std::shared_ptr<T> make(
        const ::rack::math::Vec& size,  
        MidiSequencerPtr seq, 
        std::function<void(bool)> dismisser);

};