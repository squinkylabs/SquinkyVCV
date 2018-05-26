
#include "Squinky.hpp"
#include "WidgetComposite.h"

/**
 * Implementation class for BootyModule
 */
struct bModule : Module
{
    bModule();

    /**
     * Overrides of Module functions
     */
    void step() override;

private:
    typedef float T;

};

extern float values[];
extern const char* ranges[];

bModule::bModule() : Module(0,0,0,0)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    //shifter.init();
}


void bModule::step()
{
 
}

////////////////////
// module widget
////////////////////

struct bWidget : ModuleWidget
{
    bWidget(bModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
bWidget::bWidget(bModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

   
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
Model *modelBModule = Model::create<bModule, bWidget>("Squinky Labs",
    "squinkylabs-p",
    "b", EFFECT_TAG);

