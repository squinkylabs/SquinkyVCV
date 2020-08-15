
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "Basic.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqTooltips.h"
#include "ctrl/SqWidgets.h"

using Comp = Basic<WidgetComposite>;

/**
 */
struct BasicModule : Module
{
public:
    BasicModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;
    std::shared_ptr<Comp> basic;
private:
};

void BasicModule::onSampleRateChange()
{
}


class WaveformParamQuantity : public SqTooltips::SQParamQuantity {
public:
    WaveformParamQuantity(const ParamQuantity& other) : SqTooltips::SQParamQuantity(other) {}
    std::string getDisplayValueString() override {
        const Comp::Waves wf = Comp::Waves (std::round(getValue()));
        return Comp::getLabel(wf);
    }
};

BasicModule::BasicModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    basic = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    SqTooltips::changeParamQuantity<WaveformParamQuantity>(this, Comp::WAVEFORM_PARAM);

    onSampleRateChange();
    basic->init();
}

void BasicModule::process(const ProcessArgs& args)
{
    basic->process(args);
}

////////////////////
// module widget
////////////////////

struct BasicWidget : ModuleWidget
{
    BasicWidget(BasicModule *);
    DECLARE_MANUAL("Blank Manul", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(BasicModule *module, std::shared_ptr<IComposite> icomp);
    void addControls(BasicModule *module, std::shared_ptr<IComposite> icomp);
};

void BasicWidget::addJacks(BasicModule *module, std::shared_ptr<IComposite> icomp)
{
    addInput(createInput<PJ301MPort>(
        Vec(10, 290),
        module,
        Comp::VOCT_INPUT));

    addOutput(createOutput<PJ301MPort>(
        Vec(10, 330),
        module,
        Comp::MAIN_OUTPUT));
};

void BasicWidget::addControls(BasicModule *module, std::shared_ptr<IComposite> icomp)
{
    const float knobX = 12;
    const float knobY = 12;
    const float dy = 39;

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX, knobY + 0 * dy),
        module,  Comp::OCTAVE_PARAM));
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX, knobY + 1 * dy),
        module,  Comp::SEMI_PARAM));
     addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 2 * dy),
        module,  Comp::FINE_PARAM));

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX, knobY + 3 * dy),
        module,  Comp::WAVEFORM_PARAM));
     addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX, knobY + 4 * dy),
        module,  Comp::FM_PARAM));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

BasicWidget::BasicWidget(BasicModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/basic-panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);
    addControls(module, icomp);

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelBasicModule = createModel<BasicModule, BasicWidget>("squinkylabs-basic");


