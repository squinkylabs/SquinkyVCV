
#include <sstream>

#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SAMP
#include <osdialog.h>

#include "InstrumentInfo.h"
#include "Samp.h"
#include "SqStream.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/TextDisplay.h"

using Comp = Samp<WidgetComposite>;

/** SampModule
 * Audio processing module for sfz player
 */
struct SampModule : Module {
public:
    SampModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> samp;

    void setNewSamples(const FilePath& fp) {
        // later we might change samp to also take file path..
        samp->setNewSamples_UI(fp.toString());
        lastSampleSetLoaded = fp.toString();
    }

    void setSamplePath(const std::string& s) {
        samp->setSamplePath_UI(s);
    }

    void setKeySwitch(int pitch) {
        samp->setKeySwitch_UI(pitch);
    }

    float getProgressPct() {
        return samp->getProgressPct();
    }

    InstrumentInfoPtr getInstrumentInfo();
    bool isNewInstrument();

    void dataFromJson(json_t* data) override;
    json_t* dataToJson() override;

    std::string deserializedPath;
    std::string lastSampleSetLoaded;

private:
};

const char* sfzpath = "sfzpath";

void SampModule::dataFromJson(json_t* rootJ) {
    json_t* pathJ = json_object_get(rootJ, sfzpath);
    if (pathJ) {
        const char* path = json_string_value(pathJ);
        std::string sPath(path);
        deserializedPath = sPath;
    }
}

json_t* SampModule::dataToJson() {
    json_t* rootJ = json_object();
    if (!lastSampleSetLoaded.empty()) {
        json_object_set_new(rootJ, sfzpath, json_string(lastSampleSetLoaded.c_str()));
    }
    return rootJ;
}

InstrumentInfoPtr SampModule::getInstrumentInfo() {
    return samp->getInstrumentInfo_UI();
}

bool SampModule::isNewInstrument() {
    return samp->isNewInstrument_UI();
}

void SampModule::onSampleRateChange() {
}

SampModule::SampModule() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    samp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);

    onSampleRateChange();
    samp->init();
}

void SampModule::process(const ProcessArgs& args) {
    samp->process(args);
}

////////////////////////////////////////////////////////////
// module widget
// UI for sfz player
////////////////////////////////////////////////////////////

#define _TW

 static const char* helpUrl = "https://github.com/squinkylabs/SquinkyVCV/blob/mar/docs/sfz-player.md";
//static const char* helpUrl = "https://docs.google.com/document/d/1u0aNMgU48jRmy7Hd8WDtvvvUDQd9pOlNtknvWKs5qf0";

struct SampWidget : ModuleWidget {
    SampWidget(SampModule* m);
    void appendContextMenu(Menu* theMenu) override {
        ::rack::ui::MenuLabel* spacerLabel = new ::rack::ui::MenuLabel();
        theMenu->addChild(spacerLabel);
        ManualMenuItem* manual = new ManualMenuItem("Samp manual", helpUrl);
        theMenu->addChild(manual);

        {
            SqMenuItem* sfile = new SqMenuItem(
                []() { return false; },
                [this]() { this->loadSamplerFile(); });
            sfile->text = "Load Sample file";
            theMenu->addChild(sfile);
        }
        {
            SqMenuItem* test = new SqMenuItem(
                []() { return false; },
                [this]() { this->debug(); });
            test->text = "Debug Test";
            theMenu->addChild(test);
        }
#if 0  // add the root folder
        {
            SqMenuItem* spath = new SqMenuItem(
                []() { return false; },
                [this]() { this->getRootFolder(); });
            spath->text = "Set default sample path";
            theMenu->addChild(spath);
        }
#endif
    }

    void step() override;
    void debug();

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void loadSamplerFile();
    void getRootFolder();
    void addJacks(SampModule* module, std::shared_ptr<IComposite> icomp);
    void addKnobs(SampModule* module, std::shared_ptr<IComposite> icomp);

#if 0
    void setSamplePath(const std::string& s) {
        _module->setSamplePath(s);
        pathLabel->text = s;
    }
#endif

    void requestNewSampleSet(const FilePath& s);

    SampModule* _module;

    std::vector<int> keySwitchForIndex;
    int lastKeySwitchSent = -1;

    /************************************************************************************** 
     * Stuff related to UI state and implementing it
     */
    enum class State { Empty,
                       Loading,
                       Loaded,
                       Error,
                       Initial };
    State curUIState = State::Initial;
    State nextUIState = State::Empty;
    PopupMenuParamWidget* keyswitchPopup = {nullptr};

    // display labels. they change as state changes

    TextDisplaySamp* textField;
    //	textField = createWidget<LedDisplayTextField>(mm2px(Vec(3.39962, 14.8373)));

    InstrumentInfoPtr info;
    std::string curBaseFileName;

    void pollForStateChange();
    void pollNewState();
    void pollForProgress();
    void pollForDeserializedPatch();
    void updateUIForEmpty();
    void updateUIForLoading();
    void updateUIForLoaded();
    void updateUIForError();

    void removeKeyswitchPopup();
    void buildKeyswitchUI();
    std::string buildPitchrangeUIString();

    float curProgress = 0;
};

const float leftSide = 10;
const float text1y = 70;
const float text2y = 100;
const float keyswitchy = 150;

void SampWidget::requestNewSampleSet(const FilePath& fp) {
    curBaseFileName = fp.getFilenamePartNoExtension();
    _module->setNewSamples(fp);
    nextUIState = State::Loading;
}

void SampWidget::updateUIForEmpty() {
    textField->setText("No SFZ file loaded.");
}

void SampWidget::updateUIForLoading() {
    float pct = _module->getProgressPct();
    SqStream str;
    str.add("Loading ");
    str.add(curBaseFileName);
    str.add("...\n");
    str.add("Progress: ");
    str.add(int(pct));
    textField->setText(str.str());
}

void SampWidget::updateUIForError() {
    std::string s = "Error: ";
    if (info) {
        s += info->errorMessage;
    }
    textField->setText(s);
}

void SampWidget::pollForStateChange() {
    if (_module && _module->isNewInstrument()) {
        info = _module->getInstrumentInfo();
        nextUIState = info->errorMessage.empty() ? State::Loaded : State::Error;
    }
}

void SampWidget::pollNewState() {
    if (nextUIState != curUIState) {
        removeKeyswitchPopup();
        // INFO("found ui state change. going to %d", nextUIState);
        switch (nextUIState) {
            case State::Empty:
                updateUIForEmpty();
                break;
            case State::Loaded:
                updateUIForLoaded();
                break;
            case State::Loading:
                updateUIForLoading();
                break;
            case State::Error:
                updateUIForError();
                break;
            default:
                WARN("UI state changing to %d, not imp", nextUIState);
        }
        curUIState = nextUIState;
    }
}

inline void SampWidget::removeKeyswitchPopup() {
    if (keyswitchPopup) {
        removeChild(keyswitchPopup);
        keyswitchPopup = nullptr;
    }
}

void SampWidget::step() {
    ModuleWidget::step();
    pollForDeserializedPatch();
    pollForStateChange();
    pollNewState();
    pollForProgress();
}

void SampWidget::pollForProgress() {
    if (curUIState == State::Loading) {
        int oldProgress = curProgress;
        curProgress = _module->getProgressPct();
        if (int(curProgress) != oldProgress) {
            updateUIForLoading();
        }
    }
}

void SampWidget::pollForDeserializedPatch() {
    if (!_module) {
        return;
    }

    const bool empty = _module->deserializedPath.empty();
    if (!empty) {
        FilePath fp(_module->deserializedPath);
        _module->deserializedPath.clear();
        requestNewSampleSet(fp);
    }
}

void SampWidget::updateUIForLoaded() {
    std::string s = "Samples: ";
    s += curBaseFileName;
    s += "\n";
    s += buildPitchrangeUIString();
    textField->setText(s);
    // now the ks stuff
    buildKeyswitchUI();
}

void SampWidget::buildKeyswitchUI() {
    keySwitchForIndex.clear();
    if (!info->keyswitchData.empty()) {
        std::vector<std::string> labels;
        if (info->defaultKeySwitch < 0) {
            labels.push_back("(no default key switch)");
            keySwitchForIndex.push_back(-1);
        }
        for (auto it : info->keyswitchData) {
            labels.push_back(it.first);
            const int pitch = it.second.first;
            if (pitch != it.second.second) {
                SQWARN("skipping ks range > 1");
            }
            keySwitchForIndex.push_back(pitch);
        }

        keyswitchPopup = SqHelper::createParam<PopupMenuParamWidget>(
            nullptr,
            Vec(leftSide, keyswitchy),
            _module,
            Comp::DUMMYKS_PARAM);
        keyswitchPopup->box.size.x = 160;  // width
        keyswitchPopup->box.size.y = 22;   // should set auto like button does
        keyswitchPopup->text = "noise";
        keyswitchPopup->setLabels(labels);
        addParam(keyswitchPopup);
        keyswitchPopup->setCallback([this](int index) {
            if (index < 0) {
                return;
            }
            const int pitch = keySwitchForIndex[index];
            if (pitch != lastKeySwitchSent) {
                _module->setKeySwitch(pitch);
                lastKeySwitchSent = pitch;
            }
        });
    }
}

std::string SampWidget::buildPitchrangeUIString() {
    SqStream s;
    s.add("Pitch range: ");
    s.add(info->minPitch);
    s.add("-");
    s.add(info->maxPitch);
    return s.str();
}

void SampWidget::loadSamplerFile() {
    static const char SMF_FILTERS[] = "Standard Sfz file (.sfz):sfz";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    std::string dir = "";
    DEFER({
        osdialog_filters_free(filters);
    });

    char* pathC = osdialog_file(OSDIALOG_OPEN, dir.c_str(), filename.c_str(), filters);

    if (!pathC) {
        // Fail silently
        return;
    }
    DEFER({
        std::free(pathC);
    });
    printf("load sampler got %s\n", pathC);
    fflush(stdout);

    //FATAL("finish file load");
    if (pathC) {
        lastKeySwitchSent = -1;
        this->requestNewSampleSet(FilePath(pathC));
        nextUIState = State::Loading;
    }
}

void SampWidget::getRootFolder() {
    static const char SMF_FILTERS[] = "Standard Sfz file (.sfz):sfz";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    std::string dir = "";
    DEFER({
        osdialog_filters_free(filters);
    });

    char* pathC = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), filename.c_str(), filters);

    if (!pathC) {
        // Fail silently
        return;
    }
    DEFER({
        std::free(pathC);
    });
}

void SampWidget::addJacks(SampModule* module, std::shared_ptr<IComposite> icomp) {
    float jacksY = 323;
    float jacksY0 =  jacksY - 50;
    float jacksX = 15;
    float jacksDx = 40;
    float labelY = jacksY - 25;
    float labelY0 = jacksY0 - 25;
    addLabel(
        Vec(jacksX + 0 * jacksDx - 5, labelY),
        "Out");
    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY),
        module,
        Comp::AUDIO_OUTPUT));

    addLabel(
        Vec(jacksX + 1 * jacksDx - 10, labelY),
        "V/Oct");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY),
        module,
        Comp::PITCH_INPUT));

    addLabel(
        Vec(jacksX + 2 * jacksDx - 10, labelY),
        "Gate");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 2 * jacksDx, jacksY),
        module,
        Comp::GATE_INPUT));

    addLabel(
        Vec(jacksX + 3 * jacksDx - 6, labelY),
        "Vel");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 3 * jacksDx, jacksY),
        module,
        Comp::VELOCITY_INPUT));

    //    FM_INPUT,

    addLabel(
        Vec(jacksX + 4 * jacksDx - 6, labelY),
        "FM");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 4 * jacksDx, jacksY),
        module,
        Comp::FM_INPUT));
    //  LFM_INPUT
    addLabel(
        Vec(jacksX + 5 * jacksDx - 6, labelY),
        "LFM");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 5 * jacksDx, jacksY),
        module,
        Comp::LFM_INPUT));

    addLabel(
        Vec(jacksX + 5 * jacksDx - 6, labelY0),
        "Dpth");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 5 * jacksDx, jacksY0),
        module,
        Comp::LFM_DEPTH));
}

void SampWidget::addKnobs(SampModule* module, std::shared_ptr<IComposite> icomp) {
    float knobsY = 200;
    //  float knobsX = 15;
    float knobsX = 173;
    float knobsDx = 40;

    float labelDy = 25;
    float labelY = knobsY - labelDy;

    float knobsY2 = knobsY + 40;

    addLabel(
        Vec(knobsX - 6 - knobsDx, labelY),
        "Vol");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobsX - knobsDx, knobsY),
        module,
        Comp::VOLUME_PARAM));

    addLabel(
        Vec(knobsX - 6, labelY),
        "Pitch");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobsX, knobsY),
        module,
        Comp::PITCH_PARAM));

    addLabel(
        Vec(knobsX - 6 + 1 * knobsDx, labelY),
        "Depth");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobsX + 1 * knobsDx, knobsY),
        module,
        Comp::LFM_DEPTH_PARAM));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(knobsX + 6 + +0 * knobsDx, knobsY2),
        module,
        Comp::PITCH_TRIM_PARAM));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SampWidget::SampWidget(SampModule* module) {
    setModule(module);
    _module = module;
    SqHelper::setPanel(this, "res/samp_panel.svg");

    addLabel(Vec(80, 10), "SFZ Player");

    textField = createWidget<TextDisplaySamp>(mm2px(Vec(3.39962, 14.8373)));
    textField->box.size = Vec(250, 100);
    addChild(textField);

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);
    addKnobs(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

static void shouldFindMalformed(const char* input) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go(input, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    // SQINFO("now will compile");
    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    if (!cinst) {
        SQWARN("did not compile. bailing");
        return;
    }

    if (errc.empty()) {
        SQWARN("did not find malf");
    }
}

static void testMalformedRelease() {
    shouldFindMalformed(R"foo(
        <region>ampeg_release=abcd
        )foo");
    shouldFindMalformed(R"foo(
        <region>ampeg_release=qb.3
        )foo");
}

static void testMalformedKey() {
    shouldFindMalformed(R"foo(
        <region>key=abcd
        )foo");
    shouldFindMalformed(R"foo(
        <region>key=c#
        )foo");
    shouldFindMalformed(R"foo(
        <region>key=cn
        )foo");
    shouldFindMalformed(R"foo(
        <region>key=c.
        )foo");
    shouldFindMalformed(R"foo(
        <region>key=h3
        )foo");
}

void SampWidget::debug() {
    SQINFO("start debug");
    const char* input = "12345";
    try {
        float x = std::stof(input);
        printf("converted to %f\n", x);
    } catch (std::exception&) {
        WARN("excpetion converting float");
    }

    input = "abc";
    try {
        float x = std::stof(input);
        printf("converted abc to %f\n", x);
    } catch (std::exception&) {
        WARN("excpetion converting bad float float");
    }

    testMalformedRelease();
    testMalformedKey();
    SQINFO("test finished");
}

Model* modelSampModule = createModel<SampModule, SampWidget>("squinkylabs-samp");
#endif
