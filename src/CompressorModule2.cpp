

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
#include "ctrl/SqTooltips.h"
#include "ctrl/SqVuMeter.h"
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
        INFO("Module::onReset");
        compressor->initAllParams();
    }
    //   float getGainReductionDb();

    int getNumVUChannels() {
        return compressor->getNumVUChannels();
    }
    float getChannelGain(int channel) {
        return compressor->getChannelGain(channel);
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

#ifdef _NEWTIPS
    addParams();
#else
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);

    // customize tooltips for some params.
    SqTooltips::changeParamQuantity<AttackQuantity>(this, Comp::ATTACK_PARAM);
    SqTooltips::changeParamQuantity<ReleaseQuantity>(this, Comp::RELEASE_PARAM);
    SqTooltips::changeParamQuantity<ThresholdQuantity>(this, Comp::THRESHOLD_PARAM);
    SqTooltips::changeParamQuantity<MakeupGainQuantity>(this, Comp::MAKEUPGAIN_PARAM);
    SqTooltips::changeParamQuantity<RatiosQuantity>(this, Comp::RATIO_PARAM);
    SqTooltips::changeParamQuantity<BypassQuantity>(this, Comp::NOTBYPASS_PARAM);
    SqTooltips::changeParamQuantity<WetdryQuantity>(this, Comp::WETDRY_PARAM);
#endif

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
                this->configParam<BypassQuantity2>(i, param.min, param.max, param.def, paramName);
                break;
            default:

                // module->params[i].config(param.min, param.max, param.def, paramName);
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

#define _LAB

// this control adapted from Fundamental VCA 16 channel level meter
// widget::TransparentWidget
class VCA_1VUKnob : public widget::TransparentWidget {
public:
    Compressor2Module* module;

    VCA_1VUKnob() {
        box.size = mm2px(Vec(10, 46));
    }

    void draw(const DrawArgs& args) override {
        const int numberOfSegments = 25;
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
        nvgFillColor(args.vg, nvgRGB(0, 0, 0));
        nvgFill(args.vg);

        const Vec margin = Vec(3, 3);
        const Rect r = box.zeroPos().grow(margin.neg());
        const int channels = module ? module->getNumVUChannels() : 1;

        // Segment value
        const float value = 1;
        nvgBeginPath(args.vg);
        nvgRect(args.vg,
                r.pos.x,
                r.pos.y + r.size.y * (1 - value),
                r.size.x,
                r.size.y * value);
        nvgFillColor(args.vg, color::mult(color::WHITE, 0.33));
        nvgFill(args.vg);

        // Segment gain
        nvgBeginPath(args.vg);
        for (int c = 0; c < channels; c++) {
            // gain == 1...0
            const float gain = module ? module->getChannelGain(c) : 1.f;
            // db = 0.... -infi

            // let's do 1 db per segment
            const double dbMaxReduction = -numberOfSegments;
            const double db = std::max(AudioMath::db(gain), dbMaxReduction);
            const double y0 = r.pos.y;
            const double h = db * r.size.y / dbMaxReduction;

            if (h >= 0.005f) {
                nvgRect(args.vg,
                        r.pos.x + r.size.x * c / channels,
                        y0,
                        r.size.x / channels,
                        h);
            }
        }
        const NVGcolor blue = nvgRGB(48, 125, 238);
        nvgFillColor(args.vg, blue);
        nvgFill(args.vg);

        // Invisible separators
        nvgBeginPath(args.vg);
        for (int i = 1; i <= numberOfSegments; i++) {
            nvgRect(args.vg,
                    r.pos.x - 1.0,
                    r.pos.y + r.size.y * i / float(numberOfSegments),
                    r.size.x + 2.0,
                    1.0);
        }
        nvgFillColor(args.vg, color::BLACK);
        nvgFill(args.vg);
    }
};

struct CompressorWidget2 : ModuleWidget {
    CompressorWidget2(Compressor2Module*);
    void appendContextMenu(Menu* menu) override;

#ifdef _LAB
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK) {
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
    void step() override;

    int lastStereo = -1;
    int lastChannel = -1;
    int lastLabelMode = -1;
    ParamWidget* channelKnob = nullptr;
    Label* channelIndicator = nullptr;

  //  Label* stereoLabel = nullptr;
  //  Label* channelTypeLabel = nullptr;

    Compressor2Module* const cModule;
    CompressorParamChannel pasteBuffer;

    void setAllChannelsToCurrent();
    void copy();
    void paste();
    void initializeCurrent();
};

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
    SQINFO("copy using channel %d", currentChannel);
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
    ManualMenuItem* manual = new ManualMenuItem("Comp 2 Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/c2/docs/compressor2.md");
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
#if 0
    auto itemc = new SqMenuItem_BooleanParam2(module, Comp::SIDECHAIN_ALL_PARAM);
    itemc->text = "All sidechains from channel 1";
    theMenu->addChild(itemc);
#endif

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
    //  SQINFO("in step read params st=%d lastst=%d, lavelMode=%d lastMode %d",
    //         stereo, lastStereo, labelMode, lastLabelMode);

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
      //  stereoLabel->text = Comp2TextUtil::stereoModeText(stereo);
        // INFO("set knob max to %d", (int)channelKnob->paramQuantity->maxValue);
    }

    // draw the channel label
    const int channel = int(std::round(::rack::appGet()->engine->getParam(module, Comp::CHANNEL_PARAM)));
    if ((channel != lastChannel) || (labelMode != lastLabelMode)) {
        //  SQINFO("on change, stereo = %d channel = %d", stereo, channel);

        channelIndicator->text = Comp2TextUtil::channelLabel(labelMode, channel);
    }
#if 0
    if (labelMode != lastLabelMode) {
        //  static std::string channelModeMenuLabel(int mode, int stereo);
        channelTypeLabel->text = Comp2TextUtil::channelModeMenuLabel(labelMode, stereo > 0);
    }
#endif
    lastStereo = stereo;
    lastLabelMode = labelMode;
    lastChannel = channel;
}

void CompressorWidget2::addVu(Compressor2Module* module) {
    auto vu = new VCA_1VUKnob();
    vu->box.pos = Vec(92, 160);
    vu->module = module;
    addChild(vu);
}

#define TEXTCOLOR SqHelper::COLOR_BLACK

void CompressorWidget2::addControls(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    addLabel(
        Vec(1, 200),
        "Atck", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(5, 218),
        module, Comp::ATTACK_PARAM));

#ifdef _LAB
    addLabel(
        Vec(50, 200),
        "Rel", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(52, 218),
        module, Comp::RELEASE_PARAM));

#ifdef _LAB
    addLabel(
        Vec(0, 148),
        "Thrsh", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(8, 165),
        module, Comp::THRESHOLD_PARAM));

#ifdef _LAB
    addLabel(
        Vec(40, 38),
        "Channel", TEXTCOLOR);
#endif
    channelKnob = SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(8, 41),
        module, Comp::CHANNEL_PARAM);
    addParam(channelKnob);
    channelIndicator = addLabel(Vec(62, 54), "", TEXTCOLOR);

#ifdef _LAB
    addLabel(
        Vec(5, 251),
        "Mix", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(8, 269),
        module, Comp::WETDRY_PARAM));

#ifdef _LAB
    addLabel(
        Vec(49, 148),
        "Out", TEXTCOLOR);
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(52, 165),
        module, Comp::MAKEUPGAIN_PARAM));

#ifdef _LAB
    addLabel(Vec(50, 253), "1/0", TEXTCOLOR);
#endif

  //  stereoLabel = addLabel(Vec(4, 76), "Mode:");
 //   channelTypeLabel = addLabel(Vec(4, 90), "Channels:");

    ToggleButton* tog = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(56, 273),
        module, Comp::NOTBYPASS_PARAM);
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);

    std::vector<std::string> labels = Comp::ratios();
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(8, 115),
        module,
        Comp::RATIO_PARAM);
    p->box.size.x = 73;  // width
    p->box.size.y = 22;
    p->text = labels[3];
    p->setLabels(labels);
    addParam(p);

    tog = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(90, 92),
        module, Comp::SIDECHAIN_PARAM);
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);
}

void CompressorWidget2::addJacks(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    addLabel(
        Vec(12, 307),
        "In", TEXTCOLOR);
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(11, 326),
        module,
        Comp::LAUDIO_INPUT));

#ifdef _LAB
    addLabel(
        Vec(49, 307),
        "SC", TEXTCOLOR);
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(51, 326),
        module,
        Comp::SIDECHAIN_INPUT));

#ifdef _LAB
    addLabel(
        Vec(86, 307),
        "Out", TEXTCOLOR);
#endif
    addOutput(createOutput<PJ301MPort>(
        Vec(91, 326),
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
        Vec(20, 15),
        "Compressor II", TEXTCOLOR);
#endif

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addControls(module, icomp);
    addJacks(module, icomp);
    addVu(module);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model* modelCompressor2Module = createModel<Compressor2Module, CompressorWidget2>("squinkylabs-comp2");
