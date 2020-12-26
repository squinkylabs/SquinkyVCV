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

//using SKeyValuePairPtr = std::shared_ptr<SKeyValuePair>; 
//using SKeyValueList = std::vector<SKeyValuePairPtr>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;
using SRegionPtr = std::shared_ptr<SRegion>;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;
using SGroupPtr = std::shared_ptr<SGroup>;

namespace ci
{
using CompiledInstrumentPtr = std::shared_ptr<class CompiledInstrument>;


class CompiledInstrument : public ISamplerPlayback {
public:
    static CompiledInstrumentPtr make(const SInstrumentPtr);
    void play(VoicePlayInfo&, int midiPitch, int midiVelocity) override;
    void _setTestMode() {
       // testMode = true;
        pitchMap._setTestMode();
    }

    /**
     * move all the waves from here to wave loader
     */
    void setWaves(WaveLoaderPtr waveLoader, const std::string& rootPath);

    /**
     * finds all the key/value pairs in a parse tree and expands them in place.
     */
    static void expandAllKV(SInstrumentPtr);

private:

#if 1
    PitchSwitch pitchMap;
#else
    bool testMode = false;

    // This needs to be ISamplerPlayerPtr, not VoicePlayInfoPtr 
    std::map<int, VoicePlayInfoPtr> pitchMap;
#endif

    /**
     * Track all the unique relative paths here
     * key = file path
     * value = index (wave id);
     */
    std::map<std::string, int> relativeFilePaths;    
    int nextIndex = 1;

    void compile(const SInstrumentPtr);
    void compileSub(const SRegionPtr);
    bool shouldIgnoreGroup(SGroupPtr);

    /** Returns wave index
     */
    int addSampleFile(const std::string& s);
};


//KeysAndValuesPtr compile(const SKeyValueList&);






}