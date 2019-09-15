

#include "SqKey.h"

#include "jansson.h"

#include <assert.h>
#include <sstream>

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

    char key = 0;
    bool ctrl = false;
    bool shift = false;
    while (getline(f, s, ';')) {
        if (s == "ctrl") {
            assert(!ctrl);
            ctrl = true;
        } else if (s.length() == 1) {
            key = s[0];
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

    assert(false);
    return false;
}