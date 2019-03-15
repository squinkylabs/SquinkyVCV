
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SLEW
#include "Slew4.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

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
    : Module(Slew4.NUM_PARAMS,
    Slew4.NUM_INPUTS,
    Slew4.NUM_OUTPUTS,
    Slew4.NUM_LIGHTS),
    Slew4(this)
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
    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
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
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1
Model *modelSlew4Module = createModel<Slew4Module, Slew4Widget>("squinkylabs-slew4");
#else
a b c
Model *modelSlew4Module = Model::create<Slew4Module,
    Slew4Widget>("Squinky Labs",
    "squinkylabs-slew4",
    "-- Slew4 --", RANDOM_TAG);
#endif
#endif

