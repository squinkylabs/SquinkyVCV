
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _F4
#include "F4.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

using Comp = F4<WidgetComposite>;

/**
 */
struct F4Module : Module
{
public:
    F4Module();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void F4Module::onSampleRateChange()
{
}

F4Module::F4Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();
}

void F4Module::process(const ProcessArgs& args)
{
    blank->process(args);
}

////////////////////
// module widget
////////////////////

struct F4Widget : ModuleWidget
{
    F4Widget(F4Module *);
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

    void addJacks(F4Module *module, std::shared_ptr<IComposite> icomp);
    void addKnobs(F4Module *module, std::shared_ptr<IComposite> icomp);
};

void F4Widget::addKnobs(F4Module *module, std::shared_ptr<IComposite> icomp)
{
    const float knobX = 12;
    const float knobY = 21;
    const float dy = 39;

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 0* dy),
        module,  Comp::FC_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 1* dy),
        module,  Comp::Q_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 2* dy),
        module,  Comp::R_PARAM));
}

void F4Widget::addJacks(F4Module *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float jackY = 300;
    const float dy = 30;
    const float dx = 30;

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + dy),
        module,
        Comp::AUDIO_INPUT));
#if 0
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 1 * dy),
        module,
        Comp::FM_INPUT)); 

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy),
        module,
        Comp::VOCT_INPUT));
    #endif

    addOutput(createOutput<PJ301MPort>(
        Vec(jackX + dx, jackY + dy ),
        module,
        Comp::LP_OUTPUT));
     addOutput(createOutput<PJ301MPort>(
        Vec(jackX + dx, jackY ),
        module,
        Comp::HP_OUTPUT));

    addOutput(createOutput<PJ301MPort>(
        Vec(jackX + 2* dx, jackY + dy ),
        module,
        Comp::BP_OUTPUT));
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

F4Widget::F4Widget(F4Module *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/f4-panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);
    addKnobs(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelF4Module = createModel<F4Module, F4Widget>("squinkylabs-f4");
#endif

