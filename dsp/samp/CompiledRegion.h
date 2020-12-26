#pragma once

#include <memory>
#include <string>

class SRegion;
using SRegionPtr = std::shared_ptr<SRegion>;

class CompiledRegion
{
public:
    CompiledRegion(SRegionPtr);

    int lokey = -1;
    int hikey = -1;
    int onlykey = -1;
    int keycenter = -1;
    std::string sampleFile;
    int lovel = 0;
    int hivel = 127;
};

using CompiledRegionPtr = std::shared_ptr<CompiledRegion>;
