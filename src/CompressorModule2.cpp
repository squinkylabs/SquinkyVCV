

#include "C2Json.h"
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
    //   float getGainReductionDb();

    int getNumVUChannels() {
        return compressor->getNumVUChannels();
    }
    float getChannelGain(int channel) {
        return compressor->getChannelGain(channel);
    }

    virtual json_t* dataToJson() override;
    virtual void dataFromJson(json_t* root) override;

private:
    std::shared_ptr<Comp> compressor;
};

Compressor2Module::Compressor2Module() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    compressor = std::make_shared<Comp>(this);
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

    onSampleRateChange();
    compressor->init();
}

json_t* Compressor2Module::dataToJson() {
    const CompressorParmHolder& params = compressor->getParamHolder();
    C2Json ser;
    return ser.paramsToJson(params);
}

void Compressor2Module::dataFromJson(json_t* rootJ) {
    CompressorParmHolder* params = &compressor->getParamHolder();
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
    //  int lastLabelMode = -1;
    ParamWidget* channelKnob = nullptr;
    Label* channelIndicator = nullptr;

    Label* stereoLabel = nullptr;
    Label* channelTypeLabel = nullptr;
};

void CompressorWidget2::appendContextMenu(Menu* theMenu) {
    MenuLabel* spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    ManualMenuItem* manual = new ManualMenuItem("F2 Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/f2.md");
    theMenu->addChild(manual);

    SubMenuParamCtrl::create(theMenu, "stereo/mono", {"mono", "stereo", "linked-stereo"}, module, Comp::STEREO_PARAM);

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

    SubMenuParamCtrl::create(theMenu, "panel channels", {"1-8", "9-16", "Group/Aux"}, module, Comp::LABELS_PARAM, render);
}

void CompressorWidget2::step() {
    ModuleWidget::step();
    if (!module) {
        return;
    }
    const int stereo = int(std::round(::rack::appGet()->engine->getParam(module, Comp::STEREO_PARAM)));
    const int labelMode = int(std::round(::rack::appGet()->engine->getParam(module, Comp::LABELS_PARAM)));

    if (stereo != lastStereo) {
        const int steps = stereo ? 8 : 16;
        channelKnob->paramQuantity->maxValue = steps;
        if (channelKnob->paramQuantity->getValue() > steps) {
            ::rack::appGet()->engine->setParam(module, Comp::CHANNEL_PARAM, steps);
        }

        switch (stereo) {
            case 0:
                stereoLabel->text = "Mode: multi-mono";
                break;
            case 1:
                stereoLabel->text = "Mode: stereo";
                break;
            case 2:
                stereoLabel->text = "Mode: linked-stereo";
                break;
        }
        // INFO("set knob max to %d", (int)channelKnob->paramQuantity->maxValue);
    }

    // draw the channel label
    const int channel = int(std::round(::rack::appGet()->engine->getParam(module, Comp::CHANNEL_PARAM)));
    if ((channel != lastChannel) || (labelMode != lastLabelMode)) {
        //  SQINFO("on change, stereo = %d channel = %d", stereo, channel);
        SqStream sq;
        switch (labelMode) {
            case 0:
                sq.add(channel);
                break;
            case 1:
                sq.add(channel + 8);
                break;
            case 2: {
                switch (channel) {
                    case 1:
                        sq.add("G1");
                        break;
                    case 2:
                        sq.add("G2");
                        break;
                    case 3:
                        sq.add("G3");
                        break;
                    case 4:
                        sq.add("G4");
                        break;
                    case 5:
                        sq.add("A1");
                        break;
                    case 6:
                        sq.add("A2");
                        break;
                    case 7:
                        sq.add("A3");
                        break;
                    case 8:
                        sq.add("A4");
                        break;
                    default:
                        FATAL("channel out of range %d", channel);
                        assert(false);
                }
            } break;
        }

        channelIndicator->text = sq.str();
    }

    const bool isStereo = stereo > 0;

    if (labelMode != lastLabelMode) {
        switch (labelMode) {
            case 0:
                channelTypeLabel->text = isStereo ? "Channels: 1-8" : "Channels: 1-16";
                break;
            case 1:
                channelTypeLabel->text = isStereo ? "Channels: 9-16" : "Channels: 1-16";
                break;
            case 2:
                channelTypeLabel->text = "Channels: Group/Aux";
                break;
        }
    }
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

void CompressorWidget2::addControls(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    addLabel(
        Vec(1, 200),
        "Atck");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(5, 218),
        module, Comp::ATTACK_PARAM));

#ifdef _LAB
    addLabel(
        Vec(50, 200),
        "Rel");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(52, 218),
        module, Comp::RELEASE_PARAM));

#ifdef _LAB
    addLabel(
        Vec(0, 148),
        "Thrsh");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(8, 165),
        module, Comp::THRESHOLD_PARAM));

#ifdef _LAB
    addLabel(
        Vec(40, 38),
        "Channel");
#endif
    channelKnob = SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(8, 41),
        module, Comp::CHANNEL_PARAM);
    addParam(channelKnob);
    channelIndicator = addLabel(Vec(62, 54), "");

#ifdef _LAB
    addLabel(
        Vec(5, 251),
        "Mix");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(8, 269),
        module, Comp::WETDRY_PARAM));

#ifdef _LAB
    addLabel(
        Vec(49, 148),
        "Out");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(52, 165),
        module, Comp::MAKEUPGAIN_PARAM));

#ifdef _LAB
    addLabel(Vec(50, 253), "1/0");
#endif

    stereoLabel = addLabel(Vec(4, 76), "Mode:");
    channelTypeLabel = addLabel(Vec(4, 90), "Channels:");

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
}

void CompressorWidget2::addJacks(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    addLabel(
        Vec(12, 307),
        "In");
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(11, 326),
        module,
        Comp::LAUDIO_INPUT));

#ifdef _LAB
    addLabel(
        Vec(86, 307),
        "Out");
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

CompressorWidget2::CompressorWidget2(Compressor2Module* module) {
    setModule(module);
    SqHelper::setPanel(this, "res/compressor2_panel.svg");

#ifdef _LAB
    addLabel(
        Vec(20, 15),
        "Compressor II");
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
