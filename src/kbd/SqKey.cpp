
#include "SqKey.h"
#include "../ctrl/SqHelper.h"

#include "rack.hpp"

#include <assert.h>
#include <sstream>

std::map<std::string, int> SqKey::keyString2KeyCode;

SqKeyPtr SqKey::parse(json_t* binding)
{
    json_t* keyJ = json_object_get(binding, "key");
    if (!keyJ) {
        sqWARN("Binding does not have key field");
        return nullptr;
    }
    if (!json_is_string(keyJ)) {
        sqWARN("Binding key is not a string: %s", json_dumps(keyJ, 0));
        return nullptr;
    }

    std::string keyString = json_string_value(keyJ);
    std::istringstream f(keyString);
    std::string s;    

    int key = 0;
    bool ctrl = false;
    bool shift = false;
    bool alt = true;
    while (getline(f, s, '+')) {
        if (s == "ctrl") {
            assert(!ctrl);
            ctrl = true;
        } else if (s == "shift") {
            shift = true;
        } else if (s == "alt") {
            alt = true;
        } else if ((key = parseKey(s))) {
            //
        } else {
            sqWARN("can't parse key fragment %s of %s\n", s.c_str(), keyString.c_str());
            return nullptr;
        }
    }
    if (key == 0) {
        sqWARN("binding does not have valid key: %s\n", keyString.c_str());
        return nullptr;
    }
    SqKey* r = new SqKey(key, ctrl, shift, alt);
    SqKeyPtr ret(r);
    return ret;
}

bool SqKey::operator< (const SqKey& other) const
{
   // fprintf(stderr, "key %c < %c?\n", this->key, other.key); fflush(stderr);
    if (other.key < this->key) {
        return true;
    }
    if (other.key > this->key) {
        return false;
    }
    if (!other.ctrl && this->ctrl) {
        return true;
    }
    if (other.ctrl && !this->ctrl) {
        return false;
    }
    if (!other.shift && this->shift) {
        return true;
    }
    if (other.shift && !this->shift) {
        return false;
    }
    if (!other.alt && this->alt) {
        return true;
    }
    if (other.alt && !this->alt) {
        return false;
    }

    // if they are the same, we get here
    return false;
}

int SqKey::parseKey(const std::string& key)
{
   // INFO("key = %s", key.c_str());
    if (keyString2KeyCode.empty()) {
        initMap();
    }

    // First, look in the map
    int ret = 0;
    auto it = keyString2KeyCode.find(key);
    if (it != keyString2KeyCode.end()) {
        ret = it->second;
    }

    // then look for single digit numbers and letters
    if (!ret && (key.size() == 1)) {
        int k = key[0];
        if (k >= '0' && k <= '9') {
            ret = GLFW_KEY_0 + (k - '0');
        }

        if (k >= 'a' && k <= 'z') {
            ret = GLFW_KEY_A + (k - 'a');
        }
    }

    // look for function keys
    if (!ret && (key.size() >= 2) && key[0] == 'f') {
        // f0..f9
        if (key.size() == 2) {
            const int k = key[1];
            if (k >= '1' && k <= '9') {
                ret = GLFW_KEY_F1 + (k - '1');
            }
        // f10..19
        } else if (key.size() == 3) {
            if (key[1] == '1') {
                const int k = key[2];
                if (k >= '0' && k <= '9') {
                    ret = GLFW_KEY_F10 + (k - '0');
                }
            }
        }
    }

    return ret;
}

 void SqKey::initMap()
 {
     assert(keyString2KeyCode.empty());

     keyString2KeyCode = {
        {"left", GLFW_KEY_LEFT},
        {"right", GLFW_KEY_RIGHT},
        {"up", GLFW_KEY_UP},
        {"down", GLFW_KEY_DOWN},
        {"insert", GLFW_KEY_INSERT},
        {"numpad0", GLFW_KEY_KP_0},
        {"numpad1", GLFW_KEY_KP_1},
        {"numpad2", GLFW_KEY_KP_2},
        {"numpad3", GLFW_KEY_KP_3},
        {"numpad4", GLFW_KEY_KP_4},
        {"numpad5", GLFW_KEY_KP_5},
        {"numpad6", GLFW_KEY_KP_6},
        {"numpad7", GLFW_KEY_KP_7},
        {"numpad8", GLFW_KEY_KP_8},
        {"numpad9", GLFW_KEY_KP_9},
        {"tab", GLFW_KEY_TAB},
        {"numpad_add", GLFW_KEY_KP_ADD},
        {"numpad_subtract", GLFW_KEY_KP_SUBTRACT},
        {"=", GLFW_KEY_EQUAL},
        {"[", GLFW_KEY_LEFT_BRACKET},
        {"]", GLFW_KEY_RIGHT_BRACKET},
        {"enter", GLFW_KEY_ENTER},
        {",", GLFW_KEY_COMMA},
        {".", GLFW_KEY_PERIOD},
        {"*", GLFW_KEY_KP_MULTIPLY},
        {"`", GLFW_KEY_GRAVE_ACCENT},
        {"'", GLFW_KEY_APOSTROPHE},
        {"-", GLFW_KEY_MINUS},
        {"\\", GLFW_KEY_BACKSLASH},
        {"/", GLFW_KEY_SLASH},
        {";", GLFW_KEY_SEMICOLON},
         {"-", GLFW_KEY_MINUS},
        {"delete", GLFW_KEY_DELETE},
        {"backspace", GLFW_KEY_BACKSPACE},
        {"numpad_decimal", GLFW_KEY_KP_DECIMAL},
        {"numpad_divide", GLFW_KEY_KP_DIVIDE},
        {"numpad_multiply", GLFW_KEY_KP_MULTIPLY},
        {"pageup", GLFW_KEY_PAGE_UP},
        {"pagedown", GLFW_KEY_PAGE_DOWN},
        {"end", GLFW_KEY_END},
        {"home", GLFW_KEY_HOME},
        {"escape",GLFW_KEY_ESCAPE},
        {"space",GLFW_KEY_SPACE},
        {"pausebreak",GLFW_KEY_PAUSE},
        {"capslock",GLFW_KEY_CAPS_LOCK},
     };
 }
