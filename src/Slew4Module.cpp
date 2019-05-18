
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SLEW
#include "Slew4.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

using Comp = Slew4<WidgetComposite>;

/**
 */
struct Slew4Module : Module
{
public:
    Slew4Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> slew;
private:

};

void Slew4Module::onSampleRateChange()
{
    slew->onSampleRateChange();
}


#ifdef __V1
Slew4Module::Slew4Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    slew = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
Slew4Module::Slew4Module()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS),
    slew( std::make_shared<Comp>(this))
{
#endif
    onSampleRateChange();
    slew->init();
}

void Slew4Module::step()
{
    slew->step();
}

////////////////////
// module widget
////////////////////

struct Slew4Widget : ModuleWidget
{
    Slew4Widget(Slew4Module *);
    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/sq5/docs/slew4.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(Slew4Module *);
    void addScrews();
    void addOther(Slew4Module*, std::shared_ptr<IComposite> icomp);
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
Slew4Widget::Slew4Widget(Slew4Module *module)
{
    setModule(module);
#else
Slew4Widget::Slew4Widget(Slew4Module *module) : ModuleWidget(module)
{
#endif
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/slew_panel.svg");

    addJacks(module);
    addScrews();
    addOther(module, icomp);
}

float jackY = 60;
float jackDy = 30;
float jackX = 18;
float jackDx = 28;

void Slew4Widget::addJacks(Slew4Module *module)
{
    for (int i=0; i<8; ++i) {
        addInput(createInputCentered<PJ301MPort>(
            Vec(jackX, jackY + i * jackDy),
            module,
            Comp::INPUT_TRIGGER0 + i));

        addInput(createInputCentered<PJ301MPort>(
            Vec(jackX + jackDx, jackY + i * jackDy),
            module,
            Comp::INPUT_AUDIO0 + i));

        addOutput(createOutputCentered<PJ301MPort>(
            Vec(jackX + 2 * jackDx, jackY + i * jackDy),
            module,
            Comp::OUTPUT0 + i));
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(jackX + 3 * jackDx, jackY + i * jackDy),
            module,
            Comp::OUTPUT_MIX0 + i));
    }
    addLabel(Vec(jackX -22, jackY-40), "Gate");
    addLabel(Vec(jackX + jackDx -16, jackY-40), "(In)");
    addLabel(Vec(jackX + 2 * jackDx -18, jackY-40), "Out");
    addLabel(Vec(jackX + 3 * jackDx -18, jackY-40), "Mix");
}

float knobY= 330;
float knobX = 20;
float knobDx = 36;
float labelAboveKnob = 36;

void Slew4Widget::addOther(Slew4Module*, std::shared_ptr<IComposite> icomp)
{
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(knobX, knobY),
        module,
        Comp::PARAM_RISE));
    addLabel(Vec(knobX - 20, knobY - labelAboveKnob), "Rise");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(knobX + knobDx, knobY),
        module,
        Comp::PARAM_FALL));
    addLabel(Vec(knobX + knobDx - 20, knobY - labelAboveKnob), "Fall");

     addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(knobX + 2 * knobDx, knobY),
        module,
        Comp::PARAM_LEVEL));
    addLabel(Vec(knobX + 2 * knobDx - 20, knobY - labelAboveKnob), "Level");
};

void Slew4Widget::addScrews()
{
    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1
Model *modelSlew4Module = createModel<Slew4Module, Slew4Widget>("squinkylabs-slew4");
#else
Model *modelSlew4Module = Model::create<Slew4Module,
    Slew4Widget>("Squinky Labs",
    "squinkylabs-slew4",
    "Slade: Octal slew/lag with mixing", SLEW_LIMITER_TAG);
#endif
#endif

