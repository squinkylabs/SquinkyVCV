

#include "C2Json.h"
#include "Comp2TextUtil.h"
#include "Compressor2.h"
#include "CompressorTooltips.h"
#include "SqStream.h"
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SubMenuParamCtrl.h"
#include "ctrl/ToggleButton.h"
#include "engine/Port.hpp"

using Comp = Compressor2<WidgetComposite>;
#define _NEWTIPS

/**********************************************************
 * 
 *  MODULE definition
 */
struct Compressor2Module : Module {
public:
    Compressor2Module();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;
    void onReset() override {
        compressor->initAllParams();
    }
    //   float getGainReductionDb();

    int getNumVUChannels() {
        return compressor->ui_getNumVUChannels();
    }
    float getChannelGain(int channel) {
        return compressor->ui_getChannelGain(channel);
    }

    virtual json_t* dataToJson() override;
    virtual void dataFromJson(json_t* root) override;

    std::shared_ptr<Comp> compressor;

private:
    void addParams();
};

Compressor2Module::Compressor2Module() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    compressor = std::make_shared<Comp>(this);

    addParams();

    onSampleRateChange();
    compressor->init();
}

void Compressor2Module::addParams() {
    std::shared_ptr<IComposite> comp = Comp::getDescription();
    const int n = comp->getNumParams();
    for (int i = 0; i < n; ++i) {
        auto param = comp->getParam(i);
        std::string paramName(param.name);
        switch (i) {
            case Comp::ATTACK_PARAM:
                this->configParam<AttackQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            case Comp::RELEASE_PARAM:
                this->configParam<ReleaseQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            case Comp::MAKEUPGAIN_PARAM:
                this->configParam<MakeupGainQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            case Comp::THRESHOLD_PARAM:
                this->configParam<ThresholdQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            case Comp::WETDRY_PARAM:
                this->configParam<WetdryQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            case Comp::RATIO_PARAM:
                this->configParam<RatiosQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            case Comp::NOTBYPASS_PARAM:
                this->configParam<BypassQuantityComp2>(i, param.min, param.max, param.def, paramName);
                break;
              case Comp::SIDECHAIN_PARAM:
                this->configParam<SideChainQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            default:
                this->configParam(i, param.min, param.max, param.def, paramName);
        }
    }
}

json_t* Compressor2Module::dataToJson() {
    const CompressorParamHolder& params = compressor->getParamHolder();
    C2Json ser;
    return ser.paramsToJson(params);
}

void Compressor2Module::dataFromJson(json_t* rootJ) {
    CompressorParamHolder* params = &compressor->getParamHolder();
    C2Json ser;
    ser.jsonToParams(rootJ, params);
    compressor->updateAllChannels();
}

void Compressor2Module::process(const ProcessArgs& args) {
    compressor->process(args);
}

void Compressor2Module::onSampleRateChange() {
    compressor->onSampleRateChange();
}

/*****************************************************************************

    Module widget

******************************************************************************/

#include "MultiVUMeter.h"       // need to include this after module definition

// #define _LAB
struct CompressorWidget2 : ModuleWidget {
    CompressorWidget2(Compressor2Module*);
    void appendContextMenu(Menu* menu) override;

#if 1
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_WHITE) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
#endif

    void addJacks(Compressor2Module* module, std::shared_ptr<IComposite> icomp);
    void addControls(Compressor2Module* module, std::shared_ptr<IComposite> icomp);
    void addVu(Compressor2Module* module);
   // void addNumbers();
    void step() override;

    int lastStereo = -1;
    int lastChannel = -1;
    int lastLabelMode = -1;
    ParamWidget* channelKnob = nullptr;
    Label* channelIndicator = nullptr;
    Compressor2Module* const cModule;
    CompressorParamChannel pasteBuffer;

    void setAllChannelsToCurrent();
    void copy();
    void paste();
    void initializeCurrent();
    // void updateVULabels(int stereo, int labelMode);
};

#define TEXTCOLOR SqHelper::COLOR_WHITE

void CompressorWidget2::initializeCurrent() {
    cModule->compressor->ui_initCurrentChannel();
}

void CompressorWidget2::setAllChannelsToCurrent() {
    if (module) {
        cModule->compressor->ui_setAllChannelsToCurrent();
    }
}

void CompressorWidget2::copy() {
    CompressorParamChannel ch;
    const CompressorParamHolder& params = cModule->compressor->getParamHolder();
    int currentChannel = -1 + int(std::round(::rack::appGet()->engine->getParam(module, Comp::CHANNEL_PARAM)));
    if (lastStereo > 1) {
        currentChannel *= 2;
    }
    //SQINFO("copy using channel %d", currentChannel);
    ch.copyFromHolder(params, currentChannel);
    C2Json json;
    json.copyToClip(ch);
}

void CompressorWidget2::paste() {
    C2Json json;
    bool b = json.getClipAsParamChannel(&pasteBuffer);
    if (b && module) {
        cModule->compressor->ui_paste(&pasteBuffer);
    }
}

void CompressorWidget2::appendContextMenu(Menu* theMenu) {
    MenuLabel* spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    ManualMenuItem* manual = new ManualMenuItem("Comp II manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/compressor2.md");
    theMenu->addChild(manual);

    theMenu->addChild(new SqMenuItem(
        "Copy channel",
        []() {
            return false;  // we are never checked
        },
        [this]() {
            this->copy();
        }));
    theMenu->addChild(new SqMenuItem(
        "Paste channel",
        []() {
            return false;  //TODO: enable when clip
        },
        [this]() {
            this->paste();
        }));
    spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    theMenu->addChild(new SqMenuItem(
        "Set all channels to current",
        []() {
            return false;  //TODO: enable when clip
        },
        [this]() {
            this->setAllChannelsToCurrent();
        }));
    theMenu->addChild(new SqMenuItem(
        "Initialize current channel",
        []() {
            return false;  //TODO: enable when clip
        },
        [this]() {
            this->initializeCurrent();
        }));

    SubMenuParamCtrl::create(theMenu, "Stereo/mono", {"Mono", "Stereo", "Linked-stereo"}, module, Comp::STEREO_PARAM);

    auto render = [this](int value) {
        const bool isStereo = ::rack::appGet()->engine->getParam(this->module, Comp::STEREO_PARAM) > .5;
        std::string text;
        switch (value) {
            case 0:
                text = isStereo ? "1-8" : "1-16";
                break;
            case 1:
                text = isStereo ? "9-16" : "1-16";
                break;
            case 2:
                text = "Group/Aux";
                break;
        }
        return text;
    };

    std::vector<std::string> submenuLabels;
    if (lastStereo > 0) {
        submenuLabels = {"1-8", "9-16", "Group/Aux"};
    }

    auto item = SubMenuParamCtrl::create(theMenu, "Panel channels", submenuLabels, module, Comp::LABELS_PARAM, render);
    if (lastStereo == 0) {
        item->disabled = true;
    }
}

void CompressorWidget2::step() {
    ModuleWidget::step();
    if (!module) {
        return;
    }
    const int stereo = int(std::round(::rack::appGet()->engine->getParam(module, Comp::STEREO_PARAM)));
    int labelMode = int(std::round(::rack::appGet()->engine->getParam(module, Comp::LABELS_PARAM)));

    if (stereo == 0) {
        if (labelMode != 0) {
            ::rack::appGet()->engine->setParam(module, Comp::LABELS_PARAM, 0);
            labelMode = 0;
            SQWARN("UI ignoring label mode incompatible with mono stereo=%d mode=%d", stereo, labelMode);
        }
    }

    if (stereo != lastStereo) {
        const int steps = stereo ? 8 : 16;
        channelKnob->paramQuantity->maxValue = steps;
        if (channelKnob->paramQuantity->getValue() > steps) {
            ::rack::appGet()->engine->setParam(module, Comp::CHANNEL_PARAM, steps);
        }
    }

    // draw the channel label
    const int channel = int(std::round(::rack::appGet()->engine->getParam(module, Comp::CHANNEL_PARAM)));
    if ((channel != lastChannel) || (labelMode != lastLabelMode)) {
        channelIndicator->text = Comp2TextUtil::channelLabel(labelMode, channel);
    }
    lastStereo = stereo;
    lastLabelMode = labelMode;
    lastChannel = channel;
}

void CompressorWidget2::addVu(Compressor2Module* module) {
    auto vu = new MultiVUMeter(&lastStereo, &lastLabelMode, &lastChannel);
    vu->box.pos = Vec(5, 83);
    vu->module = module;
    addChild(vu);

    auto lab = new VULabels(&lastStereo, &lastLabelMode, &lastChannel, !module);
    lab->box.pos = Vec(5, 73);
    addChild(lab);
}

// from kitchen sink
class SqBlueButton : public ToggleButton {
public:
    SqBlueButton() {
        addSvg("res/oval-button-up-grey.svg");
        addSvg("res/oval-button-down.svg");
    }

};

class SqBlueButtonInv : public ToggleButton {
public:
    SqBlueButtonInv() {
        addSvg("res/oval-button-down.svg");
        addSvg("res/oval-button-up-grey.svg");
    }
};

void CompressorWidget2::addControls(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    addLabel(
        Vec(52, 193),
        "Att", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(52, 211),
        module, Comp::ATTACK_PARAM));

#ifdef _LAB
    addLabel(
        Vec(96, 193),
        "Rel", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(98, 211),
        module, Comp::RELEASE_PARAM));

#ifdef _LAB
    addLabel(
        Vec(2, 193),
        "Thrsh", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(6, 211),
        module, Comp::THRESHOLD_PARAM));

#ifdef _LAB
    addLabel(
        Vec(54, 35),
        "Channel:", TEXTCOLOR);
#endif
    channelKnob = SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(17, 24),
        module, Comp::CHANNEL_PARAM);
    addParam(channelKnob);
    // was 104 / 35
    // then 96 / 32
    // then 92, 31

    channelIndicator = addLabel(Vec(93, 31.8), "", TEXTCOLOR);

#ifdef _LAB
    addLabel(
        Vec(4, 250),
        "Mix", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(6, 268),
        module, Comp::WETDRY_PARAM));

#ifdef _LAB
    addLabel(
        Vec(93, 250),
        "Out", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(98, 268),
        module, Comp::MAKEUPGAIN_PARAM));

#ifdef _LAB
    addLabel(Vec(49, 250), "Ena", TEXTCOLOR);
#endif
    SqBlueButtonInv* enaButton = SqHelper::createParam<SqBlueButtonInv>(
        icomp,
        Vec(52, 268),
        module, Comp::NOTBYPASS_PARAM);
    addParam(enaButton);

    // x = 32 too much
    std::vector<std::string> labels = Comp::ratios();
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(27, 163),
        module,
        Comp::RATIO_PARAM);
    p->box.size.x = 80;  // width
    p->box.size.y = 22;
    p->text = labels[3];
    p->setLabels(labels);
    addParam(p);

    SqBlueButton* scButton = SqHelper::createParam<SqBlueButton>(
        icomp,
        Vec(52, 304),
        module, Comp::SIDECHAIN_PARAM);
    addParam(scButton);
}

void CompressorWidget2::addJacks(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    addLabel(
        Vec(8, 308),
        "In", TEXTCOLOR);
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(9, 326),
        module,
        Comp::LAUDIO_INPUT));

#ifdef _LAB
    addLabel(
        Vec(51, 289),
        "SC", TEXTCOLOR);
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(54.5, 326),
        module,
        Comp::SIDECHAIN_INPUT));

#ifdef _LAB
    addLabel(
        Vec(95, 308),
        "Out", TEXTCOLOR);
#endif
    addOutput(createOutput<PJ301MPort>(
        Vec(101, 326),
        module,
        Comp::LAUDIO_OUTPUT));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

CompressorWidget2::CompressorWidget2(Compressor2Module* module) : cModule(module) {
    setModule(module);

    SqHelper::setPanel(this, "res/compressor2_panel.svg");

#ifdef _LAB
    addLabel(
        Vec(41, 2),
        "Comp II", TEXTCOLOR);
#endif

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addControls(module, icomp);
    addJacks(module, icomp);
    addVu(module);
}

Model* modelCompressor2Module = createModel<Compressor2Module, CompressorWidget2>("squinkylabs-comp2");
