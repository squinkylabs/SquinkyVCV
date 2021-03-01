
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
#include "ctrl/TextDisplay.h"

using Comp = Samp<WidgetComposite>;

/**
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
    }

    void setSamplePath(const std::string& s) {
        samp->setSamplePath_UI(s);
    }

    void setKeySwitch(int pitch) {
        samp->setKeySwitch_UI(pitch);
    }

    InstrumentInfoPtr getInstrumentInfo();
    bool isNewInstrument();

private:
};

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

////////////////////
// module widget
////////////////////

#define _TW

// static const char* helpUrl = "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md";
static const char* helpUrl = "https://docs.google.com/document/d/1u0aNMgU48jRmy7Hd8WDtvvvUDQd9pOlNtknvWKs5qf0";

struct SampWidget : ModuleWidget {
    SampWidget(SampModule* m);
    //  DECLARE_MANUAL("Samp Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");
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

#ifdef _TW
    TextDisplay* textField;
    //	textField = createWidget<LedDisplayTextField>(mm2px(Vec(3.39962, 14.8373)));
#else
    // Empty: "No SFZ file loaded"
    // Loading: "Loading xxx.sfz"
    // Loaded: Playing xxx.sfz
    // Error : error message
    Label* uiText1 = {nullptr};

    // Empty: blank
    // Loading: progress
    // Loaded: pitch range
    // Error: blank
    Label* uiText2 = {nullptr};
#endif

    InstrumentInfoPtr info;
    std::string curBaseFileName;

    void pollForStateChange();
    void pollNewState();
    void updateUIForEmpty();
    void updateUIForLoading();
    void updateUIForLoaded();
    void updateUIForError();

    void removeKeyswitchPopup();
    void buildKeyswitchUI();
    std::string buildPitchrangeUIString();
};

const float leftSide = 20;
const float text1y = 70;
//const float text1Height = 40;
const float text2y = 100;
const float keyswitchy = 50;

void SampWidget::requestNewSampleSet(const FilePath& fp) {
    curBaseFileName = fp.getFilenamePart();
    _module->setNewSamples(fp);
    nextUIState = State::Loading;
}

void SampWidget::updateUIForEmpty() {
#ifndef _TW
    uiText1->text = "No SFZ file loaded.";
    uiText2->text = "";
#else
    textField->text = "No SFZ file loaded.";
#endif
}

void SampWidget::updateUIForLoading() {
#ifdef _TW
    std::string s = "Loading ";
    s += curBaseFileName;
    s += "...";
    textField->text = s;
#else
    INFO("in loading, set cur to %s", curBaseFileName.c_str());
    std::string s = "Loading ";
    s += curBaseFileName;
    s += "...";
    uiText1->text = s;
    uiText2->text = "";
#endif
}

void SampWidget::updateUIForError() {
#ifdef _TW
#else
    std::string s = "Error: ";
    if (info) {
        s += info->errorMessage;
    }
    uiText1->text = s;
    uiText2->text = "";
#endif
}

void SampWidget::pollForStateChange() {
    if (_module && _module->isNewInstrument()) {
        info = _module->getInstrumentInfo();
        SQINFO("in UI, error = %s", info->errorMessage.c_str());
        SQINFO("got info there are %d labels", info->keyswitchData.size());
        nextUIState = info->errorMessage.empty() ? State::Loaded : State::Error;
    }
}

void SampWidget::pollNewState() {
    if (nextUIState != curUIState) {
        removeKeyswitchPopup();
        INFO("found ui state change. going to %d", nextUIState);
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
    pollForStateChange();
    pollNewState();
}

void SampWidget::updateUIForLoaded() {
    std::string s = "Samples: ";
    s += curBaseFileName;
    s += "\n";
    s += buildPitchrangeUIString();
#ifdef _TW
    textField->text = s;
#else
  
    uiText1->text = s;
    uiText2->text =  buildPitchrangeUIString();
#endif
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
    s.add("Pitch Range: ");
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
#if 0
        FATAL("move keyswitch removal");
        if (keyswitchPopup) {
            removeChild(keyswitchPopup);
            keyswitchPopup = nullptr;
        }
#endif
        lastKeySwitchSent = -1;
        this->requestNewSampleSet(FilePath(pathC));
        //  _module->setNewSamples(pathC);

        INFO("setting state to loading (%d)", State::Loading);
        nextUIState = State::Loading;
    }
}

void SampWidget::getRootFolder() {
    static const char SMF_FILTERS[] = "Standard Sfz file (.sfz):sfz";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    // std::string dir = _module->sequencer->context->settings()->getMidiFilePath();
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
    SQINFO("getRootFolder got %s", pathC);
    WARN("I don't think this is the rigth way to load samples");
    //   this->requestNewSampleSet(pathC);
}

void SampWidget::addJacks(SampModule* module, std::shared_ptr<IComposite> icomp) {
    float jacksY = 340;
    float jacksX = 15;
    float jacksDx = 40;
    float labelY = jacksY - 25;

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
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SampWidget::SampWidget(SampModule* module) {
    setModule(module);
    _module = module;
    SqHelper::setPanel(this, "res/blank_panel.svg");

    addLabel(Vec(80, 10), "-Sample Player-");

#ifdef _TW
    textField = createWidget<TextDisplay>(mm2px(Vec(3.39962, 14.8373)));
    //		textField->box.size = mm2px(Vec(74.480, 102.753));
    textField->box.size = Vec(250, 100);
    textField->multiline = true;
    addChild(textField);
#else
    uiText1 = addLabel(Vec(leftSide, text1y), "");
    uiText2 = addLabel(Vec(leftSide, text2y), "");
#endif

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model* modelSampModule = createModel<SampModule, SampWidget>("squinkylabs-samp");
#endif
