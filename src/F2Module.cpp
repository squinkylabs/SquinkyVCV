
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _F2
#include "F2.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

using Comp = F2<WidgetComposite>;

/**
 */
struct F2Module : Module
{
public:
    F2Module();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void F2Module::onSampleRateChange()
{
}

F2Module::F2Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();
}

void F2Module::process(const ProcessArgs& args)
{
    blank->process(args);
}

////////////////////
// module widget
////////////////////

struct F2Widget : ModuleWidget
{
    F2Widget(F2Module *);
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

    void addJacks(F2Module *module, std::shared_ptr<IComposite> icomp);
    void addKnobs(F2Module *module, std::shared_ptr<IComposite> icomp);
};

void F2Widget::addKnobs(F2Module *module, std::shared_ptr<IComposite> icomp)
{
    const float knobX = 12;
    const float knobY = 21;
    const float dy = 39;
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX, knobY + 0 * dy),
        module,  Comp::TOPOLOGY_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 1* dy),
        module,  Comp::FC_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 2* dy),
        module,  Comp::Q_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 3* dy),
        module,  Comp::R_PARAM));
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX, knobY + 4* dy),
        module,  Comp::MODE_PARAM));
}

void F2Widget::addJacks(F2Module *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float jackY = 249;
    const float dy = 30;

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 0 * dy),
        module,
        Comp::AUDIO_INPUT));

    addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy - .5),
        module,
        Comp::AUDIO_OUTPUT));
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

F2Widget::F2Widget(F2Module *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/f2-panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);
    addKnobs(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelF2Module = createModel<F2Module, F2Widget>("squinkylabs-f2");
#endif

