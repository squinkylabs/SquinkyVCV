

#include "TextUtils.h"
#include "SqLog.h"

FontPtr TextUtils::loadFont(WhichFont f) {
    std::string fontPath("res/fonts/");
    switch(f) {
        case WhichFont::regular:
            fontPath += "Roboto-Regular.ttf";
            break;
         case WhichFont::bold:
            fontPath += "Roboto-Bold.ttf";
            break;
        default:
            assert(false);
            return ::rack::appGet()->window->uiFont;
    }
    return APP->window->loadFont(asset::plugin(pluginInstance, fontPath.c_str()));
}