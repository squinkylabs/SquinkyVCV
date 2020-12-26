
#include "CompiledInstrument.h"
#include "CompiledRegion.h"

#include "SParse.h"

#include <set>
#include <functional>

using Opcode = SamplerSchema::Opcode;
CompiledRegion::CompiledRegion(SRegionPtr region) 
{
    const SRegion& reg = *region;
    assert(reg.compiledValues);

    // TODO: better API for getting values
    auto value = reg.compiledValues->get(SamplerSchema::Opcode::LO_KEY);
    if (value) {
        lokey = value->numericInt;
    }
    value = reg.compiledValues->get(Opcode::HI_KEY);
    if (value) {
        hikey = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::SAMPLE);
    if (value) {
        assert(!value->string.empty());
        sampleFile = value->string;
    }
            
    value = reg.compiledValues->get(Opcode::PITCH_KEYCENTER);
    if (value) {
        keycenter = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::LO_VEL);
    if (value) {
        lovel = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::HI_VEL);
    if (value) {
        hivel = value->numericInt;
    }

 
}

#if 0
void CompiledInstrument::compileSub(const SRegionPtr region)
{
    const SRegion& reg = *region;
         
    int lokey = -1;
    int hikey = -1;
    int onlykey = -1;
    int keycenter = -1;
    std::string sampleFile;
    int lowvel = 0;
    int hivel = 127;

    printf("compile Sub\n");
// this may not scale ;-)
    auto value = reg.compiledValues->get(Opcode::LO_KEY);
    if (value) {
        lokey = value->numericInt;
    }
    value = reg.compiledValues->get(Opcode::HI_KEY);
    if (value) {
        hikey = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::SAMPLE);
    if (value) {
        assert(!value->string.empty());
        sampleFile = value->string;
    }
            
    value = reg.compiledValues->get(Opcode::PITCH_KEYCENTER);
    if (value) {
        keycenter = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::LO_VEL);
    if (value) {
        lowvel = value->numericInt;
    }

    value = reg.compiledValues->get(Opcode::HI_VEL);
    if (value) {
        hivel = value->numericInt;
    }

    // until we do vel switching, just grab 64
    if ((lowvel > 64) || (hivel < 64)) {
        // printf("rejecting vel layer %d,%d\n", lowvel, hivel); fflush(stdout);
        return;
    }
     printf("compile Sub 2\n");

    if ((lokey >= 0) && (hikey >= 0) && !sampleFile.empty()) {

         printf("compile Sub 3\n");
        const int sampleIndex = addSampleFile(sampleFile);
        for (int key = lokey; key <= hikey; ++key) {
            VoicePlayInfoPtr info = std::make_shared< VoicePlayInfo>();
            info->valid = true;
            // need to add sample index, transpose amount, etc...
     
            info->sampleIndex = sampleIndex;
            if (key != keycenter && (keycenter != -1)) {
                // we really would like the sample rate info here!

                //  float amount = float(key) / float(keycenter);
                int semiOffset = key - keycenter;
                float pitchMul = float(std::pow(2, semiOffset / 12.0));
                // printf("just added amount = %f key = %d, center = %d\n", pitchMul, key, keycenter);
                info->needsTranspose = true;
                info->transposeAmt = pitchMul;
            } else {
                info->needsTranspose = false;
                info->transposeAmt = 1;
            }
            //   printf("faking sample index 1\n");
            //  printf("adding entry for pitch %d, si=%d\n", key, sampleIndex);

            // it we over-write something bad will happen
            auto temp = pitchMap.find(key);
            assert(temp == pitchMap.end());

            pitchMap[key] = info;
                
        }
    }
    else {
        printf("region defined nothing\n");
    } 
}
#endif


#if 0

//std::set<CompiledRegionPtr, compr> s;

auto s = std::set<int, std::function<bool(const int&, const int&)>>{
    [](const int& a, const int& b)
        {
            return a < b;
        }
};
#endif