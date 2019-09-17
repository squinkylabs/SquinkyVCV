

#include "SqKey.h"

#include "jansson.h"
#include <GLFW/glfw3.h>

#include <assert.h>
#include <sstream>


std::map<std::string, int> SqKey::keyString2KeyCode;

SqKeyPtr SqKey::parse(json_t* binding)
{
    json_t* keyJ = json_object_get(binding, "key");
    if (!keyJ) {
        fprintf(stderr, "binding does not have key field: %s\n",
            json_dumps(keyJ, 0));
        return nullptr;
    }
    if (!json_is_string(keyJ)) {
        fprintf(stderr, "binding key is not a string");
        return nullptr;
    }

    std::string keyString = json_string_value(keyJ);
    std::istringstream f(keyString);
    std::string s;    

    int key = 0;
    bool ctrl = false;
    bool shift = false;
    while (getline(f, s, '+')) {
        if (s == "ctrl") {
            assert(!ctrl);
            ctrl = true;
        } else if (s == "shift") {
            shift = true;
        } else if ((key = parseKey(s))) {
            //
        } else {
            fprintf(stderr, "can't parse key fragment %s of %s\n", s.c_str(), keyString.c_str());
            return nullptr;
        }
    }
    if (key == 0) {
        fprintf(stderr, "binding does not have valid key: %s\n", keyString.c_str());
    }
    SqKey* r = new SqKey(key, ctrl, shift);
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
    // fprintf(stderr, "b key %c < %c?\n", this->key, other.key); fflush(stderr);
    if (!other.ctrl && this->ctrl) {
        return true;
    }
    if (other.ctrl && !this->ctrl) {
        return false;
    }
   //  fprintf(stderr, "c key %c < %c?\n", this->key, other.key); fflush(stderr);
     if (!other.shift && this->shift) {
        return true;
    }
    if (other.shift && !this->shift) {
        return false;
    }

    // if they are the sam, we get ere
    return false;
}

int SqKey::parseKey(const std::string& key)
{
    if (keyString2KeyCode.empty()) {
        initMap();
    }

    int ret = 0;
    auto it = keyString2KeyCode.find(key);
    if (it != keyString2KeyCode.end()) {
        ret = it->second;
    }

    if (!ret && (key.size() == 1)) {
        int k = key[0];
        if (k >= '0' && k <= '9') {
            ret = GLFW_KEY_0 + (k - '0');
        }

        if (k >= 'a' && k <= 'z') {
            ret = GLFW_KEY_A + (k - 'a');
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
        {"f1", GLFW_KEY_F1},
        {"tab", GLFW_KEY_TAB},
        {"numpad_add", GLFW_KEY_KP_ADD},
        {"numpad_subtract", GLFW_KEY_KP_SUBTRACT},
        {"=", GLFW_KEY_EQUAL},
        {"[", GLFW_KEY_LEFT_BRACKET},
        {"]", GLFW_KEY_RIGHT_BRACKET},
        {"enter", GLFW_KEY_ENTER},
        {",", GLFW_KEY_COMMA},
        {".", GLFW_KEY_PERIOD}
     };
 }
