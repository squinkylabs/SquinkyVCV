
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _BLANKMODULE
#include "Blank.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Blank<WidgetComposite>;

/**
 */
struct BlankModule : Module
{
public:
    BlankModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void BlankModule::onSampleRateChange()
{
}


#ifdef __V1
BlankModule::BlankModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
BlankModule::BlankModule()
    : Module(blank.NUM_PARAMS,
    blank.NUM_INPUTS,
    blank.NUM_OUTPUTS,
    blank.NUM_LIGHTS),
    blank(this)
{
#endif
    onSampleRateChange();
    blank->init();
}

void BlankModule::step()
{
    blank->step();
}

////////////////////
// module widget
////////////////////

struct BlankWidget : ModuleWidget
{
    BlankWidget(BlankModule *);
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
BlankWidget::BlankWidget(BlankModule *module)
{
    setModule(module);
#else
BlankWidget::BlankWidget(BlankModule *module) : ModuleWidget(module)
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
Model *modelBlankModule = createModel<BlankModule, BlankWidget>("squinkylabs-blank");
#else
a b c
Model *modelBlankModule = Model::create<BlankModule,
    BlankWidget>("Squinky Labs",
    "squinkylabs-blank",
    "-- Blank --", RANDOM_TAG);
#endif
#endif

