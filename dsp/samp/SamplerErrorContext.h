#pragma once

#include "SqLog.h"
#include <set>


class SamplerErrorContext {
public:
    bool empty() const {
        return unrecognizedOpcodes.empty();
    }
    void dump() const {
        //SQWARN("err dump nimp");
        if (!unrecognizedOpcodes.empty()) {
            SQINFO("unimplemented opcodes:");
        }
        for (auto x : unrecognizedOpcodes) {
            SQINFO("%s", x.c_str());
        }
    }
    std::set<std::string> unrecognizedOpcodes;
};
