#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FilePath.h"
//#include "PitchSwitch.h"
#include "RegionPool.h"
#include "SamplerPlayback.h"

class FilePath;
class SInstrument;
class SRegion;
class WaveLoader;
class SGroup;
class SamplerErrorContext;
class InstrumentInfo;

using SInstrumentPtr = std::shared_ptr<SInstrument>;
using SRegionPtr = std::shared_ptr<SRegion>;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;
using SGroupPtr = std::shared_ptr<SGroup>;
using CompiledInstrumentPtr = std::shared_ptr<class CompiledInstrument>;
using InstrumentInfoPtr = std::shared_ptr<InstrumentInfo>;
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
    enum class Tests {
        None,
        MiddleC,   // PLays sample 1 at midi pitch 60 rel = .6
        MiddleC11,   // PLays sample 1 at midi pitch 60 rel = 1.1
    };

    /**
     * high level entry point to compile an instrument.
     * will return null if error, and log the error cause as best it can.
     * 
     * TODO: resolve InstrumentInfo vs. SamplerErrorContext. what goes is which one?
     * I'm thinking get rid of error context and put all in info?
     */
    static CompiledInstrumentPtr make(SamplerErrorContext&, const SInstrumentPtr);
    static CompiledInstrumentPtr make(const std::string& parseError);

    // returns true if caused keyswitch
    bool play(VoicePlayInfo&, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) override;
    void _dump(int depth) const override;
    void _setTestMode(Tests t) {
        ciTestMode = t;
    }

    /**
     * move all the waves from here to wave loader
     */
    void setWaves(WaveLoaderPtr waveLoader, const FilePath& rootPath);

    FilePath getDefaultPath() const { return defaultPath; }

    /**
     * finds all the key/value pairs in a parse tree and expands them in place.
     */
    static void expandAllKV(SamplerErrorContext&, SInstrumentPtr);

    int removeOverlaps(std::vector<CompiledRegionPtr>&);
    const RegionPool& _pool() { return regionPool; }
    InstrumentInfoPtr getInfo() { return info; }

    static float velToGain1(int midiVelocity, float veltrack);
    static float velToGain2(int midiVelocity, float veltrack);
    static float velToGain(int midiVelocity, float veltrack);

    bool isInError() const { return _isInError; }
private:
    RegionPool regionPool;
    Tests ciTestMode = Tests::None;
    InstrumentInfoPtr info;

    AudioMath::RandomUniformFunc rand = AudioMath::random();

    FilePath defaultPath;
    bool _isInError = false;

    /**
     * Track all the unique relative paths here
     * key = file path
     * value = index (wave id);
     * 
     * We could more properly store these as FilePath,
     * but string is convenient and fast for maps.
     * as long as do convert these immediately to FilePath it will be fine.
     */
    std::map<std::string, int> relativeFilePaths;
    int nextIndex = 1;

    bool compile(const SInstrumentPtr);
    bool compileOld(const SInstrumentPtr);
    bool fixupOneRandomGrouping(int groupStartIndex);

    /** Returns wave index
     */
    int addSampleFile(const std::string& s);
    void addSampleIndexes();
    void deriveInfo();

    /**
     * these helpers help fill in VoicePlayInfo
     */
    static void getPlayPitch(VoicePlayInfo& info, int midiPitch, int regionKeyCenter, int tuneCents, WaveLoader* loader, float sampleRate);
    static void getGain(VoicePlayInfo& info, int midiVelocity, float regionVeltrack, float regionVolumeDb);

    bool playTestMode(VoicePlayInfo&, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate);
};
