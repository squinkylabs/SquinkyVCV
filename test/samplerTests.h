#pragma once

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SamplerPlayback.h"

class st {
public:
    static CompiledRegionPtr makeRegion(const std::string& s) {
        SInstrumentPtr inst = std::make_shared<SInstrument>();
        //const char* str = R"foo(<region>sample=a lorand=0 hirand=1)foo";
        auto err = SParse::go(s.c_str(), inst);

        SGroupPtr group = inst->groups[0];
        SRegionPtr region = group->regions[0];
        CompiledInstrument::expandAllKV(inst);
        assert(inst->wasExpanded);
        CompiledRegionPtr cr = std::make_shared<CompiledRegion>(region, nullptr, group);
        return cr;
    }
};
