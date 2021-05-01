
#include "SqStream.h"
#include "Squinky.hpp"
#include "WidgetComposite.h"

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
#include "ctrl/ToggleButton.h"

using Comp = Compressor2<WidgetComposite>;

/**
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

    std::shared_ptr<Comp> compressor;
    int getNumChannels() {
        return compressor->getNumChannels();
    }
    float getChannelGain(int channel) {
        return compressor->getChannelGain(channel);
    }

private:
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

#if 0
float Compressor2Module::getGainReductionDb()
{
    return compressor->getGainReductionDb();
}
#endif

void Compressor2Module::process(const ProcessArgs& args) {
    compressor->process(args);
}

void Compressor2Module::onSampleRateChange() {
    compressor->onSampleRateChange();
}

////////////////////
// module widget
////////////////////

#define _LAB

#if 1
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
        Rect r = box.zeroPos().grow(margin.neg());

        int channels = module ? module->getNumChannels() : 1;

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

            // INFO("ch=%d gain=%f", c, gain);
            if (h >= 0.005f) {
                // INFO("c=%d va = %.2f gain=%.2f", c, gain);
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

#endif

struct CompressorWidget2 : ModuleWidget {
    CompressorWidget2(Compressor2Module*);
    DECLARE_MANUAL("Comp Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/compressor.md");

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
};

void CompressorWidget2::addVu(Compressor2Module* module) {
    // VCA_1VUKnob* levelParam = createChild<VCA_1VUKnob>(Vec(90, 40), module, Comp::RELEASE_PARAM);
    //	levelParam->module = module;
    //	addChild(levelParam);
    auto vu = new VCA_1VUKnob();
    vu->box.pos = Vec(90, 190);
    vu->module = module;
    addChild(vu);

#if 0
    auto vu = new SqVuMeter();
    vu->box.size = Vec(72, 14);
    //vu->box.pos = Vec(10, 254);
    vu->box.pos = Vec(9, 82),
    vu->setGetter( [module]() {
        return module ? module->getGainReductionDb() : 4;
    });
    addChild(vu);
#endif
}

void CompressorWidget2::addControls(Compressor2Module* module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    const float knobX = 10;
    const float knobX2 = 50;
    const float knobX3 = 90;
    // const float knobY = 58;
    // const float labelY = knobY - 20;
    // const float dy = 56;
#endif

#ifdef _LAB
    addLabel(
        Vec(knobX - 4, 174 - 20),
        "Atck");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX, knobY + 0 * dy),
        Vec(8, 174),
        module, Comp::ATTACK_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX2 - 1, 174 - 20),
        "Rel");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX2, knobY + 0 * dy),
        Vec(52, 174),
        module, Comp::RELEASE_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX - 10, 121 - 20),
        "Thrsh");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        // Vec(knobX, knobY + 1 * dy),
        Vec(8, 121),
        module, Comp::THRESHOLD_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX3 - 10, 101 - 20),
        "Chan");
#endif
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        // Vec(knobX, knobY + 1 * dy),
        Vec(knobX3, 101),
        module, Comp::CHANNEL_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX - 2, 225 - 20),
        "Mix");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX2, knobY + 1 * dy),
        Vec(8, 225),
        module, Comp::WETDRY_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX2 - 10, 121 - 20),
        "Out");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX, knobY + 2 * dy),
        Vec(52, 121),
        module, Comp::MAKEUPGAIN_PARAM));

#ifdef _LAB
    addLabel(Vec(knobX2, 229 - 20), "1/0");
#endif
#if 0
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(knobX2 + 8, 4 + knobY + 2 * dy),
        module,  Comp::BYPASS_PARAM));
#else
    ToggleButton *tog = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(55, 229),
        module, Comp::NOTBYPASS_PARAM);
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);
#endif

    std::vector<std::string> labels = Comp::ratios();
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        //Vec(knobX,  - 11 + knobY + 3 * dy),
        Vec(8, 50),
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
    const float jackX = 10;
    const float jackX2 = 50;
    const float labelX = jackX - 6;
    const float label2X = jackX2 - 6;

    const float jackY = 288 + 44;
    const float labelY = jackY - 18;

    addLabel(
        Vec(labelX + 4, labelY),
        "In");
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(11, jackY),
        module,
        Comp::LAUDIO_INPUT));

#if 0
#ifdef _LAB
    addLabel(
        Vec(labelX+4, labelY + 1 * dy),
        "InR");
#endif
    addInput(createInput<PJ301MPort>(
       // Vec(jackX, jackY + 1 * dy),
        Vec(11, 323),
        module,
        Comp::RAUDIO_INPUT));
#endif

#ifdef _LAB
    addLabel(
        Vec(label2X - 2, labelY),
        "Out");
#endif
    addOutput(createOutput<PJ301MPort>(
        //Vec(jackX2, jackY + 0 * dy),
        Vec(55, jackY),
        module,
        Comp::LAUDIO_OUTPUT));

#if 0
#ifdef _LAB
    addLabel(
        Vec(label2X - 2, labelY + 1 * dy),
        "OutR");
#endif
    addOutput(createOutput<PJ301MPort>(
       // Vec(jackX2, jackY + 1 * dy),
        Vec(55, 323),
        module,
        Comp::RAUDIO_OUTPUT));
#endif
#if 0
    addLabel(
        Vec(labelX, labelY + 2 * dy),
        "dbg");
     addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy),
        module,
        Comp::DEBUG_OUTPUT));
#endif
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
        Vec(4, 17),
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
