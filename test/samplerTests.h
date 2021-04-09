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
            SQWARN("unable to test source: %s", err.c_str());
            return nullptr;
        }

       // SGroupPtr group = inst->groups[0];
      //  SRegionPtr region = group->regions[0];
        SHeadingPtr group = nullptr;
        SHeadingPtr region = nullptr;
        if (inst->headings.size() > 0) {
            auto h0 = inst->headings[0];
            if (h0->type == SHeading::Type::Group) {
                group = h0;
            } else if (h0->type == SHeading::Type::Region) {
                region = h0;
            }

            if (!region && inst->headings.size() > 1) {
                auto h1 = inst->headings[1];
                if (h1->type == SHeading::Type::Region) {
                    region = h1;
                }
            }
        }
        assert(region);
        assert(inst->headings.size() <= 2);


        SamplerErrorContext errc;
        CompiledInstrument::expandAllKV(errc, inst);
        assert(inst->wasExpanded);
        assert(err.empty());
        assert(errc.unrecognizedOpcodes.empty());

        CompiledRegionPtr cr = std::make_shared<CompiledRegion>(23);        // pass fake line number

        if (group) {
            cr->addRegionInfo(group->compiledValues);
        }
        cr->addRegionInfo(region->compiledValues);
        return cr;
    }
};
