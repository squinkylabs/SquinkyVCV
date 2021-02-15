
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

    void setNewSamples(const std::string& s) {
        samp->setNewSamples_UI(s);
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
    void setSamplePath(const std::string& s) {
        _module->setSamplePath(s);
        pathLabel->text = s;
    }

    SampModule* _module;
    Label* pathLabel = nullptr;
    PopupMenuParamWidget* keyswitchPopup = {nullptr};
    Label* pitchRangeLabel = {nullptr};
    std::vector<int> keySwitchForIndex;
    int lastKeySwitchSent = -1;
};

const float leftSide = 20;

void SampWidget::step() {
    ModuleWidget::step();
    if (_module && _module->isNewInstrument()) {
        auto info = _module->getInstrumentInfo();
        SQINFO("got info there are %d labels", info->keyswitchData.size());

        if (!info->keyswitchData.empty()) {
            keySwitchForIndex.clear();
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
                SQINFO("pushing pitch %d", pitch);
                keySwitchForIndex.push_back(pitch);
            }

            keyswitchPopup = SqHelper::createParam<PopupMenuParamWidget>(
                nullptr,
                Vec(leftSide, 80),
                _module,
                Comp::DUMMYKS_PARAM);
            keyswitchPopup->box.size.x = 160;  // width
            keyswitchPopup->box.size.y = 22;   // should set auto like button does
            keyswitchPopup->text = "noise";
            keyswitchPopup->setLabels(labels);
            addParam(keyswitchPopup);
            keyswitchPopup->setCallback( [this](int index) {
                SQINFO("w00t! in callback index=%d", index);
                if (index != lastKeySwitchSent) {
                    _module->setKeySwitch(index);
                    lastKeySwitchSent = index;
                }
            });
        }

        SqStream s;
        s.add("Pitch Range: ");
        s.add(info->minPitch);
        s.add("-");
        s.add(info->maxPitch);
        pitchRangeLabel->text = s.str();
    }
}

void SampWidget::loadSamplerFile() {
    static const char SMF_FILTERS[] = "Standard Sfz file (.sfz):sfz";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    // std::string dir = _module->sequencer->context->settings()->getMidiFilePath();
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
    printf("got %s\n", pathC);
    fflush(stdout);

    FATAL("finish file load");
    if (pathC) {
        if (keyswitchPopup) {
            removeChild(keyswitchPopup);
            keyswitchPopup = nullptr;
        }
        lastKeySwitchSent = -1;
        _module->setNewSamples(pathC);
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
    SQINFO("got %s", pathC);
    this->setSamplePath(pathC);
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

    addLabel(Vec(leftSide, 50), "Sssssss");
    pitchRangeLabel = addLabel(Vec(leftSide, 150), "");
    pathLabel = addLabel(Vec(leftSide, 70), "");

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
