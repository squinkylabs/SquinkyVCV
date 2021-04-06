#pragma once

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SamplerPlayback.h"
#include "SamplerErrorContext.h"

class st {
public:
    static CompiledRegionPtr makeRegion(const std::string& s) {
        SInstrumentPtr inst = std::make_shared<SInstrument>();
        auto err = SParse::go(s.c_str(), inst);
        if (!err.empty()) {
            SQWARN("unable to parse file: %s", err.c_str());
            return nullptr;
        }

        SGroupPtr group = inst->groups[0];
        SRegionPtr region = group->regions[0];
        SamplerErrorContext errc;
        CompiledInstrument::expandAllKV(errc, inst);
        assert(inst->wasExpanded);
        assert(err.empty());
        assert(errc.unrecognizedOpcodes.empty());

        CompiledRegionPtr cr = std::make_shared<CompiledRegion>(23);        // pass fake line number
        cr->addRegionInfo(group->compiledValues);
        cr->addRegionInfo(region->compiledValues);
        return cr;
    }
};
