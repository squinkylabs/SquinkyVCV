
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIX8
#include "Mix8.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Mix8<WidgetComposite>;

/**
 */
struct Mix8Module : Module
{
public:
    Mix8Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> Mix8;
private:

};

void Mix8Module::onSampleRateChange()
{
}


#ifdef __V1
Mix8Module::Mix8Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
Mix8Module::Mix8Module()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS)
{
#endif
    Mix8 = std::make_shared<Comp>(this);
    onSampleRateChange();
    Mix8->init();
}

void Mix8Module::step()
{
    Mix8->step();
}

////////////////////
// module widget
////////////////////

struct Mix8Widget : ModuleWidget
{
    Mix8Widget(Mix8Module *);
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
Mix8Widget::Mix8Widget(Mix8Module *module)
{
    setModule(module);
#else
Mix8Widget::Mix8Widget(Mix8Module *module) : ModuleWidget(module)
{
#endif
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/mix8_panel.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1
Model *modelMix8Module = createModel<Mix8Module, Mix8Widget>("squinkylabs-mix8");
#else
Model *modelMix8Module = Model::create<Mix8Module,
    Mix8Widget>("Squinky Labs",
    "squinkylabs-mix8",
    "-- Mix8 --", RANDOM_TAG);
#endif
#endif

