
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _F2
#include "F2.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
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
    const float knobX2 = 45;
    const float knobY = 40;
    const float labelY = knobY - 20;
    const float dy = 50;

    addLabel(
        Vec(knobX, labelY + 0 * dy),
        "Fc");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 0 * dy),
        module,  Comp::FC_PARAM));

    addLabel(
        Vec(knobX2, labelY + 0 * dy),
        "Q");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY + 0 * dy),
        module,  Comp::Q_PARAM));
    
    addLabel(
        Vec(knobX, labelY + 1 * dy),
        "R");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 1 * dy),
        module,  Comp::R_PARAM));

    addLabel(
        Vec(knobX2 - 5, labelY + 1 * dy),
        "Limit");
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(knobX2 + 10, 5 + knobY + 1 * dy),
        module,  Comp::LIMITER_PARAM));

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX,  knobY + 2 * dy),
        module,
        Comp::MODE_PARAM);
    p->box.size.x = 54;  // width
    p->box.size.y = 22;   
    p->text = "LP";
    p->setLabels( {"LP", "HP", "BP", "N"});
    addParam(p);

    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX,  knobY + 3 * dy),
        module,
        Comp::TOPOLOGY_PARAM);
    p->box.size.x = 54;  // width
    p->box.size.y = 22;   
    p->text = "12dB";
    p->setLabels( {"12Db", "24dB", "Par"});
    addParam(p);
}

void F2Widget::addJacks(F2Module *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float jackX2 = 48;

    const float jackY = 220;
    const float dy = 36;

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 1 * dy),
        module,
        Comp::FC_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy),
        module,
        Comp::Q_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jackX2, jackY + 2 * dy),
        module,
        Comp::R_INPUT));

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 3 * dy),
        module,
        Comp::AUDIO_INPUT));

    addOutput(createOutput<PJ301MPort>(
        Vec(jackX2, jackY + 3 * dy - .5),
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

