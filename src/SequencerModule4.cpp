
#include "Sequencer4Module.h"

#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SEQ4
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Seq4<WidgetComposite>;

void Sequencer4Module::onSampleRateChange()
{
}

Sequencer4Module::Sequencer4Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    seq4Comp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    seq4Comp->init();
}

void Sequencer4Module::step()
{
    seq4Comp->step();
}

////////////////////
// module widget
////////////////////

struct Sequencer4Widget : ModuleWidget
{
    Sequencer4Widget(Sequencer4Module *);
    DECLARE_MANUAL("Blank Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/booty-shifter.md");

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

Sequencer4Widget::Sequencer4Widget(Sequencer4Module *module)
{
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelSequencer4Module = createModel<Sequencer4Module, Sequencer4Widget>("squinkylabs-sequencer4");
#endif

