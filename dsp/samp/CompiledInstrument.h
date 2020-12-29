#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "PitchSwitch.h"
#include "SamplerPlayback.h"

//class SKeyValuePair;
class SInstrument;
class SRegion;
class WaveLoader;
class SGroup;

using SInstrumentPtr = std::shared_ptr<SInstrument>;
using SRegionPtr = std::shared_ptr<SRegion>;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;
using SGroupPtr = std::shared_ptr<SGroup>;

using CompiledInstrumentPtr = std::shared_ptr<class CompiledInstrument>;

class CompiledInstrument : public ISamplerPlayback {
public:
    static CompiledInstrumentPtr make(const SInstrumentPtr);
    void play(VoicePlayInfo&, int midiPitch, int midiVelocity) override;
    void _setTestMode() {
      testMode = true;
        //pitchMap._setTestMode();
    }

    /**
     * move all the waves from here to wave loader
     */
    void setWaves(WaveLoaderPtr waveLoader, const std::string& rootPath);

    /**
     * finds all the key/value pairs in a parse tree and expands them in place.
     */
    static void expandAllKV(SInstrumentPtr);

    void getAllRegions(std::vector<CompiledRegionPtr>&);
    static void sortByVelocity(std::vector<CompiledRegionPtr>&);
    static void sortByPitch(std::vector<CompiledRegionPtr>&);

    // test accessor
    const std::vector<CompiledGroupPtr>& _groups() { return groups; }
private:
    std::vector<CompiledGroupPtr> groups;
    bool testMode = false;

    ISamplerPlaybackPtr player;

    /**
     * Track all the unique relative paths here
     * key = file path
     * value = index (wave id);
     */
    std::map<std::string, int> relativeFilePaths;    
    int nextIndex = 1;

    void compile(const SInstrumentPtr);
    void compileOld(const SInstrumentPtr);
    bool shouldIgnoreGroup(SGroupPtr);
    void buildCompiledTree(const SInstrumentPtr i);

    ISamplerPlaybackPtr buildPlayerVelLayers(std::vector<CompiledRegionPtr> inputRegions, int depth);
    ISamplerPlaybackPtr buildPlayerPitchSwitch(std::vector<CompiledRegionPtr> inputRegions, int depth);
    void addSinglePitchPlayers(PitchSwitchPtr dest, CompiledRegionPtr region);

    /** Returns wave index
     */
    int addSampleFile(const std::string& s);

    ISamplerPlaybackPtr playbackMapVelocities(std::vector<CompiledRegionPtr>& entriesForPitch, int midiPitch);
};


//KeysAndValuesPtr compile(const SKeyValueList&);
