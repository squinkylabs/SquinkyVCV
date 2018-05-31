
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "Tremelo.h"

/**
 */
struct TremeloModule : Module
{
public:
    TremeloModule();


    /**
     * Overrides of Module functions
     */
    void step() override;
   Tremelo<WidgetComposite> tremelo;
private:
 
};



TremeloModule::TremeloModule()
 : Module(tremelo.NUM_PARAMS,
    tremelo.NUM_INPUTS,
    tremelo.NUM_OUTPUTS,
    tremelo.NUM_LIGHTS),
    tremelo(this)
{
    onSampleRateChange();
}



void TremeloModule::step()
{
    tremelo.step();
}

////////////////////
// module widget
////////////////////

struct TremeloWidget : ModuleWidget
{
    TremeloWidget(TremeloModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
TremeloWidget::TremeloWidget(TremeloModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    const float rowIO = 330;
    addInput(Port::create<PJ301MPort>(Vec(10, rowIO), Port::INPUT, module, module->tremelo.AUDIO_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(60, rowIO), Port::OUTPUT, module, module->tremelo.AUDIO_OUTPUT));


   
    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelTremeloModule = Model::create<TremeloModule, TremeloWidget>("Squinky Labs",
    "squinkylabs-tremelo",
    "Tremelo", EFFECT_TAG);

