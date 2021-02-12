#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "PitchSwitch.h"
#include "RegionPool.h"
#include "SamplerPlayback.h"


class SInstrument;
class SRegion;
class WaveLoader;
class SGroup;
//class VelSwitch;
class SamplerErrorContext;

using SInstrumentPtr = std::shared_ptr<SInstrument>;
using SRegionPtr = std::shared_ptr<SRegion>;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;
using SGroupPtr = std::shared_ptr<SGroup>;
//using VelSwitchPtr = std::shared_ptr<VelSwitch>;
using CompiledInstrumentPtr = std::shared_ptr<class CompiledInstrument>;

/**
 * How "Compiling" works.
 * 
 * Compilation run after a successful parse. The input is a parse tree (SInstrumentPtr).
 * The output is a fully formed "Play" tree.
 * The intermediate data is the "compiled object" tree, which is rooted ad CompiledInstrument::groups
 * 
 * expandAllKV(inst) is called on the parse tree. It turns the textual parse data, which are
 * string key value pairs into a directly accessible database (SamplerSchema::KeysAndValuesPtr), and put back
 * into the PARSE tree as compiled values.
 * 
 * Uninitialized CompiledInstrument is created.
 * 
 * CompiledInstrument::compile(SInstrumentPtr) is called.
 * 
 * CompiledInstrument::buildCompiledTree. This runs over the parse tree, and build up
 * the compiled tree rooted in CompiledInstrument::groups. This is where we find round robin
 * and random regions and combine them into a single CompiledMiltiRegion.
 * 
 * CompiledInstrument::removeOverlaps() This is the one place where we identify any regions that might
 * play at the same time (same pitch range, same velocity range). If we find an overlap, the smallest
 * region wins and the others are discarded.
 * 
 * Lastly, the final "player" is build. This starts with buildPlayerVelLayers, but it recurses alternating velocity layers and
 * pitch layers. The is special handling for RegionGroups.
 * 
 * Some notable things:
 *      the compiled tree mirrors the structure of the parse tree pretty closely, other than the multi-regions.
 *      the "player tree" does not follow that structure at all.
 * 
 * Q: where do we pruned (for example) the release samples?
 */

class CompiledInstrument : public ISamplerPlayback {
public:
    /**
     * high level entry point to compile an instrument.
     * will return null if error, and log the error cause as best it can.
     */
    static CompiledInstrumentPtr make(SamplerErrorContext&, const SInstrumentPtr);
    void play(VoicePlayInfo&, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) override;
    void _dump(int depth) const override;
    void _setTestMode() {
        testMode = true;
    }

    /**
     * move all the waves from here to wave loader
     */
    void setWaves(WaveLoaderPtr waveLoader, const std::string& rootPath);

    std::string getDefaultPath() const { return defaultPath; }

    /**
     * finds all the key/value pairs in a parse tree and expands them in place.
     */
    static void expandAllKV(SamplerErrorContext&, SInstrumentPtr);

    int removeOverlaps(std::vector<CompiledRegionPtr>&);
    const RegionPool& _pool() { return regionPool; }

private:
    RegionPool regionPool;
    bool testMode = false;

    AudioMath::RandomUniformFunc rand = AudioMath::random();

    std::string defaultPath;

    /**
     * Track all the unique relative paths here
     * key = file path
     * value = index (wave id);
     */
    std::map<std::string, int> relativeFilePaths;
    int nextIndex = 1;

    bool compile(const SInstrumentPtr);
    bool compileOld(const SInstrumentPtr);
    bool fixupOneRandomGrouping(int groupStartIndex);

    ISamplerPlaybackPtr buildPlayerVelLayers(std::vector<CompiledRegionPtr>& inputRegions, int depth);
    ISamplerPlaybackPtr buildPlayerPitchSwitch(std::vector<CompiledRegionPtr>& inputRegions, int depth);
    void addSingleRegionPitchPlayers(PitchSwitchPtr dest, CompiledRegionPtr region);
    /** add the passed player, which happens to be a vel switch,
     * to the passed destination play. Add it at every pitch where
     * it should be in the patch map.
     */
    void addVelSwitchToCoverPitchRegions(PitchSwitchPtr dest, ISamplerPlaybackPtr velSwitch, const std::vector<CompiledRegionPtr>& regions);

    /** Returns wave index
     */
    int addSampleFile(const std::string& s);
    void addSampleIndexes();

    ISamplerPlaybackPtr playbackMapVelocities(const std::vector<CompiledRegionPtr>& entriesForPitch, int midiPitch);

    /**
     * these helpers help fill in VoicePlayInfo
     */
    void getPlayPitch(VoicePlayInfo& info, int midiPitch, int regionKeyCenter, WaveLoader* loader, float sampleRate);
    void getGain(VoicePlayInfo& info, int midiVelocity, float regionVeltrack);
};

