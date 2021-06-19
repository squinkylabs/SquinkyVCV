
#include "Compressor.h"
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

using Comp = Compressor<WidgetComposite>;

/**
 */
struct CompressorModule : Module {
public:
    CompressorModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs &args) override;
    void onSampleRateChange() override;
    float getGainReductionDb();

    std::shared_ptr<Comp> compressor;

    // These two are just for local schema data
#ifdef _CMP_SCHEMA2
    json_t *dataToJson() override;
    virtual void dataFromJson(json_t *root) override;
#endif

private:
    void addParams();
    int knownSchema = 0;
};

CompressorModule::CompressorModule() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    compressor = std::make_shared<Comp>(this);
    addParams();
    onSampleRateChange();
    compressor->init();
}

void CompressorModule::addParams() {
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
                this->configParam(i, param.min, param.max, param.def, paramName);
        }
    }
}

const static char* schemaKey = "SqSchema";

#ifdef _CMP_SCHEMA2
json_t *CompressorModule::dataToJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, schemaKey, json_integer(2));
    return rootJ;
}

void CompressorModule::dataFromJson(json_t *rootJ) {

    json_t *schemaJ = json_object_get(rootJ, schemaKey);
    if (schemaJ) {
        knownSchema = json_integer_value(schemaJ);
    }
}
#endif

float CompressorModule::getGainReductionDb() {
    return compressor->getGainReductionDb();
}

void CompressorModule::process(const ProcessArgs &args) {
    assert(knownSchema == 0 || knownSchema == 2);
    // if we didn't get schema from loading patch, then we need to
    // use a "heuristic" implemented in Compressor.
    if (knownSchema == 0) {
        knownSchema = 1;        // bump up to 1 to prevent calling over and over.
        compressor->onNewPatch(0);
    }
    compressor->process(args);
}

void CompressorModule::onSampleRateChange() {
    compressor->onSampleRateChange();
}

////////////////////
// module widget
////////////////////

struct CompressorWidget : ModuleWidget {
    CompressorWidget(CompressorModule *);
    DECLARE_MANUAL("Comp Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/compressor.md");

#ifdef _LAB
    Label *addLabel(const Vec &v, const char *str, const NVGcolor &color = SqHelper::COLOR_BLACK) {
        Label *label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
#endif

    void addJacks(CompressorModule *module, std::shared_ptr<IComposite> icomp);
    void addControls(CompressorModule *module, std::shared_ptr<IComposite> icomp);
    void addVu(CompressorModule *module);
};

void CompressorWidget::addVu(CompressorModule *module) {
    auto vu = new SqVuMeter();
    vu->box.size = Vec(72, 14);
    //vu->box.pos = Vec(10, 254);
    vu->box.pos = Vec(9, 82),
    vu->setGetter([module]() {
        return module ? module->getGainReductionDb() : 4;
    });
    addChild(vu);
}

void CompressorWidget::addControls(CompressorModule *module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    const float knobX = 10;
    const float knobX2 = 50;
    const float knobY = 58;
    const float labelY = knobY - 20;
    const float dy = 56;
#endif

#ifdef _LAB
    addLabel(
        Vec(knobX - 4, labelY + 0 * dy),
        "Atck");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX, knobY + 0 * dy),
        Vec(8, 174),
        module, Comp::ATTACK_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX2 - 1, labelY + 0 * dy),
        "Rel");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX2, knobY + 0 * dy),
        Vec(52, 174),
        module, Comp::RELEASE_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX - 10, labelY + 1 * dy),
        "Thrsh");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        // Vec(knobX, knobY + 1 * dy),
        Vec(8, 121),
        module, Comp::THRESHOLD_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX2 - 2, labelY + 1 * dy),
        "Mix");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX2, knobY + 1 * dy),
        Vec(8, 225),
        module, Comp::WETDRY_PARAM));

#ifdef _LAB
    addLabel(
        Vec(knobX - 10, labelY + 2 * dy),
        "Makeup");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        //Vec(knobX, knobY + 2 * dy),
        Vec(52, 121),
        module, Comp::MAKEUPGAIN_PARAM));

#ifdef _LAB
    addLabel(Vec(knobX2, labelY + 2 * dy), "1/0");
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
    PopupMenuParamWidget *p = SqHelper::createParam<PopupMenuParamWidget>(
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

void CompressorWidget::addJacks(CompressorModule *module, std::shared_ptr<IComposite> icomp) {
#ifdef _LAB
    const float jackX = 10;
    const float jackX2 = 50;
    const float labelX = jackX - 6;
    const float label2X = jackX2 - 6;

    const float jackY = 288;
    const float labelY = jackY - 18;
    const float dy = 44;

    addLabel(
        Vec(labelX + 4, labelY + 0 * dy),
        "InL");
#endif
    addInput(createInput<PJ301MPort>(
        //Vec(jackX, jackY),
        Vec(11, 280),
        module,
        Comp::LAUDIO_INPUT));

#ifdef _LAB
    addLabel(
        Vec(labelX + 4, labelY + 1 * dy),
        "InR");
#endif
    addInput(createInput<PJ301MPort>(
        // Vec(jackX, jackY + 1 * dy),
        Vec(11, 323),
        module,
        Comp::RAUDIO_INPUT));

#ifdef _LAB
    addLabel(
        Vec(label2X - 2, labelY + 0 * dy),
        "OutL");
#endif
    addOutput(createOutput<PJ301MPort>(
        //Vec(jackX2, jackY + 0 * dy),
        Vec(55, 280),
        module,
        Comp::LAUDIO_OUTPUT));

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

CompressorWidget::CompressorWidget(CompressorModule *module) {
    setModule(module);
    SqHelper::setPanel(this, "res/compressor_panel.svg");

#ifdef _LAB
    addLabel(
        Vec(4, 17),
        "Compressor");
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

Model *modelCompressorModule = createModel<CompressorModule, CompressorWidget>("squinkylabs-comp");
