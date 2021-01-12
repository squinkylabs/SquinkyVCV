
#include "CompiledInstrument.h"

#include <assert.h>

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

#include "CompiledRegion.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SamplerPlayback.h"
#include "VelSwitch.h"
#include "WaveLoader.h"

using Opcode = SamplerSchema::Opcode;
using OpcodeType = SamplerSchema::OpcodeType;
using DiscreteValue = SamplerSchema::DiscreteValue;
using ValuePtr = SamplerSchema::ValuePtr;
using Value = SamplerSchema::Value;

//#define _LOG
//#define _LOGOV

void CompiledInstrument::compile(const SInstrumentPtr in) {
    assert(in->wasExpanded);
    buildCompiledTree(in);

    // here we can prune the tree - removing regions that map to the same thing
    std::vector<CompiledRegionPtr> regions;
    getAllRegions(regions);
    int x = removeOverlaps(regions);
    printf("overlapts removed %d\n", x);

    // now we need to build the player tree
    player = buildPlayerVelLayers(regions, 0);
}

int CompiledInstrument::removeOverlaps(std::vector<CompiledRegionPtr>& regions) {
#ifdef _LOGOV
    printf("enter remove overlaps\n");
#endif
    int removed = 0;
    if (regions.size() < 2) {
        return removed;
    }
    sortByPitchAndVelocity(regions);
    using iterator = std::vector<CompiledRegionPtr>::iterator;
    for (iterator it = regions.begin(); it != regions.end();) {
        iterator itNext = it + 1;
        if (itNext == regions.end()) {
            return removed;
        }
        CompiledRegionPtr first = *it;
        CompiledRegionPtr second = *itNext;
#ifdef _LOGOV
        printf("overlap comparing line %d with %d\n", first->lineNumber, second->lineNumber);
        printf("  first pitch=%d,%d, vel=%d,%d\n", first->lokey, first->hikey, first->lovel, first->hivel);
        printf("  second pitch=%d,%d, vel=%d,%d\n", second->lokey, second->hikey, second->lovel, second->hivel);
        printf("  overlap pitch = %d, overlap vel = %d\n", first->overlapsPitch(*second), first->overlapsVelocity(*second));
#endif
        if (first->overlapsPitch(*second) && first->overlapsVelocity(*second)) {
            // keep the region with the smallest pitch range
            const int firstPitchRange = first->hikey - first->lokey;
            const int secondPitchRange = second->hikey - second->lokey;
            if (firstPitchRange <= secondPitchRange) {
#ifdef _LOGOV
                printf("about to erase region from %d based on conflict from %d\n", second->lineNumber, first->lineNumber);
#endif
                // if we want to erase the second one, do that.
                // it still points at first, but next iteration there will be a different next;
                regions.erase(itNext);
                ++removed;
            } else {
#ifdef _LOGOV
                printf("about to(b) erase region from %d\n", first->lineNumber);
#endif
                // we erase the first one, leaving
                // it pointing at next.
                // so we are set up to continue loop fine
                it = regions.erase(it);
            }
        } else {
            ++it;
#ifdef _LOGOV
            printf("not removing\n");
#endif
        }
    }
    return removed;
}

/** build up the tree using the original algorithm that worked for small piano
 * we don't need structure here, to flattened region list is find
 */

class RegionBin {
public:
    int loVal = -1;
    int hiVal = -1;
    std::vector<CompiledRegionPtr> regions;
};

static void dumpRegions(const std::vector<CompiledRegionPtr>& inputRegions) {
    int x = 0;
    for (auto reg : inputRegions) {
        printf("    reg[%d] #%d pitch=%d,%d vel=%d,%d\n", x, reg->lineNumber, reg->lokey, reg->hikey, reg->lovel, reg->hivel);
        ++x;
    }
}

ISamplerPlaybackPtr CompiledInstrument::buildPlayerVelLayers(std::vector<CompiledRegionPtr>& inputRegions, int depth) {
    std::vector<RegionBin> bins;
#ifdef _LOG
    printf("enter buildPlayerVelLayers depth = %d numRegsions = %d\n", depth, int(inputRegions.size()));
    dumpRegions(inputRegions);
#endif

    assert(depth < 3);
    ++depth;

    sortByVelocity(inputRegions);

    int currentBin = -1;
    int currentRegion = 0;
    int velStart = -1;
    int velEnd = -1;

    for (auto it = inputRegions.begin(); it != inputRegions.end(); ++it) {
        auto reg1 = *it;
        ++it;
        if (it == inputRegions.end()) {
            break;
        }
        auto reg2 = *it;
        if (reg1->overlapsVelocityButNotEqual(*reg2)) {
            // unevel vel layers, must skip to pith
            printf("vel layers not matched - will fall back to pitch division\n");
            return buildPlayerPitchSwitch(inputRegions, depth - 1);
        }
    }

    for (int currentRegion = 0; currentRegion < inputRegions.size(); ++currentRegion) {
        CompiledRegionPtr reg = inputRegions[currentRegion];
        // are we at a new bin?
        if (reg->lovel != velStart) {
            bins.push_back(RegionBin());
            currentBin++;
            assert(bins.size() == currentBin + 1);
            bins.back().loVal = reg->lovel;
            bins.back().hiVal = reg->hivel;
            bins.back().regions.push_back(inputRegions[currentRegion]);

            velStart = reg->lovel;
            velEnd = reg->hivel;
        } else {
            // if vel regsions are not the same, will have to do vel after pitch
            // assert( bins.back().hiVal == inputRegions[currentRegion]->hivel);
            if (bins.back().hiVal == inputRegions[currentRegion]->hivel) {
                bins.back().regions.push_back(inputRegions[currentRegion]);
            } else {
                printf("vel layers not matched - will fall back to pitch division\n");
                assert(false);  // this is from the old way
                return buildPlayerPitchSwitch(inputRegions, depth);
            }
        }
    }

    if (bins.empty()) {
        // emit a null player (pitch switch knows how)
        assert(inputRegions.empty());
        return buildPlayerPitchSwitch(inputRegions, depth);
    } else if (bins.size() == 1) {
        assert(bins[0].regions.size() == inputRegions.size());
        if (bins[0].regions.size() == 1) {
            // emit a simple player by calling pitch helper (who can emit singles
            printf("single entry in single zone\n");
            return buildPlayerPitchSwitch(bins[0].regions, depth);
        } else {
            // single bin with multiple entries
            // emit a key switch
            // one vel zone with multiple regions
            printf("multiple entry in single zone - retry pitch first\n");
            return buildPlayerPitchSwitch(bins[0].regions, depth - 1);
        }
    } else {
        // emit a vel switch and recurse
        VelSwitchPtr velSwitch = std::make_shared<VelSwitch>(bins[0].regions[0]->lineNumber);
        for (auto bin : bins) {
            ISamplerPlaybackPtr pitchPlayer = buildPlayerPitchSwitch(bin.regions, depth);

            //   void addVelocityRange(unsigned int velRangeStart, ISamplerPlaybackPtr player);
            velSwitch->addVelocityRange(bin.loVal, pitchPlayer);
        }
        return velSwitch;
    }

    assert(false);
    return nullptr;
}

void CompiledInstrument::addSingleRegionPitchPlayers(PitchSwitchPtr dest, CompiledRegionPtr region) {
    for (int midiPitch = region->lokey; midiPitch <= region->hikey; ++midiPitch) {
        switch (region->type()) {
            case CompiledRegion::Type::Base: {
                const int sampleIndex = addSampleFile(region->sampleFile);
                assert(region->type() == CompiledRegion::Type::Base);
                ISamplerPlaybackPtr singlePlayer = std::make_shared<SimpleVoicePlayer>(region, sampleIndex, midiPitch);
                dest->addEntry(midiPitch, singlePlayer);
            } break;
            case CompiledRegion::Type::Random: {
                CompiledMultiRegionPtr multiRegion = std::dynamic_pointer_cast<CompiledMultiRegion>(region);
                RandomVoicePlayerPtr multiPlayer = std::make_shared<RandomVoicePlayer>();
#ifdef _LOG
                printf("addSingleRegionPitchPlayers making Random from region #%d number in rotaion is %d\n", region->lineNumber, (int) multiRegion->getRegions().size());
#endif
                for (auto region : multiRegion->getRegions()) {
                    const int sampleIndex = addSampleFile(region->sampleFile);
#ifdef _LOG
                    printf("  adding sub-region p=%f,%f\n", region->lorand, region->hirand);
#endif
                    multiPlayer->addEntry(region, sampleIndex, midiPitch);
                }
                multiPlayer->finalize();
                dest->addEntry(midiPitch, multiPlayer);
            } break;
            case CompiledRegion::Type::RoundRobin: {
                CompiledMultiRegionPtr multiRegion = std::dynamic_pointer_cast<CompiledMultiRegion>(region);
                RoundRobinVoicePlayerPtr multiPlayer = std::make_shared<RoundRobinVoicePlayer>();
                for (auto region : multiRegion->getRegions()) {
                    const int sampleIndex = addSampleFile(region->sampleFile);
                    multiPlayer->addEntry(region, sampleIndex, midiPitch);
                }
                multiPlayer->finalize();
                dest->addEntry(midiPitch, multiPlayer);
            } break;
            default:

                assert(false);
        }
    }
}

void CompiledInstrument::addVelSwitchToCoverPitchRegions(PitchSwitchPtr dest, ISamplerPlaybackPtr velSwitch, const std::vector<CompiledRegionPtr>& regions) {
    assert(regions.size() >= 2);
    // I'm pretty sure that this gets called with all pitch regions mapping to the same pitch
    using RegionIterator = std::vector<CompiledRegionPtr>::const_iterator;
    for (RegionIterator it = regions.begin(); it != regions.end(); ++it) {
        RegionIterator it2 = it + 1;
        if (it2 == regions.end())
            break;

        CompiledRegionPtr r1 = *it;
        CompiledRegionPtr r2 = *it;
        assert(r1->pitchRangeEqual(*r2));
    }

    CompiledRegionPtr r = regions[0];
    for (int midiPitch = r->lokey; midiPitch <= r->hikey; ++midiPitch) {
        dest->addEntry(midiPitch, velSwitch);
    }
}

ISamplerPlaybackPtr CompiledInstrument::buildPlayerPitchSwitch(std::vector<CompiledRegionPtr>& inputRegions, int depth) {
#ifdef _LOG
    printf("enter buildPlayerPitchSwitch depth = %d numRegsions = %d\n", depth, int(inputRegions.size()));
    dumpRegions(inputRegions);
#endif
    assert(depth < 3);
    ++depth;
    sortByPitch(inputRegions);

    // do trivial cases:
    if (inputRegions.empty()) {
        return std::make_shared<NullVoicePlayer>();
    }

    PitchSwitchPtr playerToReturn = std::make_shared<PitchSwitch>(inputRegions[0]->lineNumber);

    if (inputRegions.size() == 1) {
        addSingleRegionPitchPlayers(playerToReturn, inputRegions[0]);
        return playerToReturn;
    }

    std::vector<RegionBin> bins;
    int currentBin = -1;
    int currentRegion = 0;
    int keyStart = -1;
    int keyEnd = -1;

    for (int currentRegion = 0; currentRegion < inputRegions.size(); ++currentRegion) {
        CompiledRegionPtr reg = inputRegions[currentRegion];
        // are we at a new bin?
        if (reg->lokey != keyStart) {
            bins.push_back(RegionBin());
            currentBin++;
            assert(bins.size() == currentBin + 1);
            bins.back().loVal = reg->lokey;
            bins.back().hiVal = reg->hikey;
            bins.back().regions.push_back(inputRegions[currentRegion]);

            keyStart = reg->lokey;
            keyEnd = reg->hikey;
        } else {
            // if vel regsions are not the same???
            assert(bins.back().hiVal == inputRegions[currentRegion]->hikey);
            bins.back().regions.push_back(inputRegions[currentRegion]);
        }
    }

    for (auto bin : bins) {
        // if this pitch bin only has one region, then we can emit it directly
        if (bin.regions.size() == 1) {
            addSingleRegionPitchPlayers(playerToReturn, bin.regions[0]);
        } else {
            // here we are binning by pitch, but a pitch bin has more than one region.
            // our only hope is to split on pitch.
            {  // debug stuff
                CompiledRegionPtr r0 = bin.regions[0];
                CompiledRegionPtr r1 = bin.regions[1];
            }
            ISamplerPlaybackPtr velSwitch = buildPlayerVelLayers(bin.regions, depth);
            // now we need to map this be switch to every pitch that it covers.
            // make this work
            addVelSwitchToCoverPitchRegions(playerToReturn, velSwitch, bin.regions);
        }
    }

    return playerToReturn;
}

void CompiledInstrument::_dump(int depth) const {
    indent(depth);
    if (player) {
        printf("Compiled Instrument dump follows:\n\n");
        player->_dump(depth);
        indent(depth);
        printf("End compiled instrument dump\n\n");
    } else {
        printf("Compiled Instrument has nothing to dump\n");
    }
}

static void remakeTreeForMultiRegion(CompiledRegion::Type type, CompiledGroupPtr cGroup) {
    assert(cGroup->regions.size());
    // First, make the new "mega region"
    // Take all the "normal" properties from the first region ("the prototype")
    std::shared_ptr<CompiledMultiRegion> multi;
    switch (type) {
        case CompiledRegion::Type::Random:
            multi = std::make_shared<CompiledRandomRegion>(cGroup);
            break;
        case CompiledRegion::Type::RoundRobin:
            multi = std::make_shared<CompiledRoundRobinRegion>(cGroup);
            break;
        default:
            assert(false);
    }

    // validate assumptions about the schema
    CompiledRegionPtr prototype = cGroup->regions[0];
    for (auto region : cGroup->regions) {
        assert(region->lokey == prototype->lokey);
        assert(region->hikey == prototype->hikey);
        assert(region->lovel == prototype->lovel);
        assert(region->hivel == prototype->hivel);
    }

    // now get rid of all the regions that were in our group
    cGroup->regions.clear();
    assert(cGroup->regions.empty());

    // And substitute this new multi region
    cGroup->addChild(multi);
    multi->weakParent = cGroup;
}

#if 1  // original way
void CompiledInstrument::buildCompiledTree(const SInstrumentPtr in) {
    for (auto group : in->groups) {
        auto cGroup = std::make_shared<CompiledGroup>(group);
        if (!cGroup->shouldIgnore()) {
            this->groups.push_back(cGroup);
            for (auto reg : group->regions) {
                auto cReg = std::make_shared<CompiledRegion>(reg, cGroup, group);
                cGroup->addChild(cReg);
            }
            // we that the tree is build, ask the group it it's special

            const CompiledRegion::Type type = cGroup->type();
            switch (type) {
                case CompiledRegion::Type::Base:
                    // nothing to do - tree is good
                    break;
                case CompiledRegion::Type::Random:
                case CompiledRegion::Type::RoundRobin: {
                    //  cGroup->regions.clear();
                    //  CompiledRegionPtr newRegion = std::make_shared < CompiledRoundRobbinRegion>();
                    //  cGroup->regions.push_back(newRegion);
                    remakeTreeForMultiRegion(type, cGroup);
                } break;
                default:
                    assert(false);
            }
        }
    }
}
#endif

void CompiledInstrument::getAllRegions(std::vector<CompiledRegionPtr>& array) {
    assert(array.empty());
    for (auto group : groups) {
        for (auto region : group->regions) {
            array.push_back(region);
        }
    }
}

void CompiledInstrument::sortByVelocity(std::vector<CompiledRegionPtr>& array) {
    std::sort(array.begin(), array.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lovel < b->lovel) {
            less = true;
        }
        return less;
    });
}

void CompiledInstrument::sortByPitch(std::vector<CompiledRegionPtr>& array) {
    std::sort(array.begin(), array.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lokey < b->lokey) {
            less = true;
        }
        return less;
    });
}

void CompiledInstrument::sortByPitchAndVelocity(std::vector<CompiledRegionPtr>& array) {
    std::sort(array.begin(), array.end(), [](const CompiledRegionPtr a, const CompiledRegionPtr b) -> bool {
        bool less = false;
        if (a->lokey < b->lokey) {
            less = true;
        } else if (a->lokey == b->lokey) {
            less = (a->lovel < b->lovel);
        }
        return less;
    });
}

int CompiledInstrument::addSampleFile(const std::string& s) {
    int ret = 0;
    auto it = relativeFilePaths.find(s);
    if (it != relativeFilePaths.end()) {
        ret = it->second;
    } else {
        relativeFilePaths.insert({s, nextIndex});
        ret = nextIndex++;
    }
    return ret;
}

#if 0  // unused
bool CompiledInstrument::shouldIgnoreGroup(SGroupPtr group) {
    bool ignore = false;
    auto value = group->compiledValues->get(Opcode::TRIGGER);
    if (value) {
        assert(value->type == OpcodeType::Discrete);
        auto trigger = value->discrete;
        ignore = (trigger != DiscreteValue::ATTACK);
    }
    return ignore;
}
#endif

CompiledInstrumentPtr CompiledInstrument::CompiledInstrument::make(SInstrumentPtr inst) {
    assert(!inst->wasExpanded);
    expandAllKV(inst);
    CompiledInstrumentPtr instOut = std::make_shared<CompiledInstrument>();
    instOut->compile(inst);
    return instOut;
}

void CompiledInstrument::play(VoicePlayInfo& info, const VoicePlayParameter& params) {
    //  pitchMap.play(info, midiPitch, midiVelocity);
    if (!player) {
        printf("ci can't play yet\n");
        return;
    }
    player->play(info, params);
}

void CompiledInstrument::setWaves(WaveLoaderPtr loader, const std::string& rootPath) {
    std::vector<std::string> tempPaths;
    assert(!rootPath.empty());

    auto num = relativeFilePaths.size();
    tempPaths.resize(num);
    // printf("resized to %zd\n", num);
    // index is 1..
    for (auto pathEntry : relativeFilePaths) {
        std::string path = pathEntry.first;

        int waveIndex = pathEntry.second;
        //printf("in setWaves, entry has %s index = %d\n", path.c_str(), waveIndex);
        // tempPaths.resize(waveIndex);
        assert(waveIndex > 0);
        assert(!path.empty());
        tempPaths[waveIndex - 1] = path;
    }
#if 0
    printf("after fill, size of temp = %zd, started with %zd\n", tempPaths.size(), relativeFilePaths.size());

    for (auto path : tempPaths) {
        printf("temp: %s\n", path.c_str());
    }
#endif

    for (auto path : tempPaths) {
        assert(!path.empty());
        WaveLoader::makeAllSeparatorsNative(path);
        loader->addNextSample(rootPath + path);
    }
}

void CompiledInstrument::expandAllKV(SInstrumentPtr inst) {
    assert(!inst->wasExpanded);
    inst->global.compiledValues = SamplerSchema::compile(inst->global.values);
    for (auto group : inst->groups) {
        group->compiledValues = SamplerSchema::compile(group->values);
        for (auto region : group->regions) {
            region->compiledValues = SamplerSchema::compile(region->values);
        }
    }
    inst->wasExpanded = true;
}
