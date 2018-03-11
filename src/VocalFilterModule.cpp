
#include "Squinky.hpp"


#include "WidgetComposite.h"
#include "VocalFilter.h"


/**
 * Implementation class for VocalWidget
 */
struct VocalFilterModule : Module
{

    VocalFilterModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
   // json_t *toJson() override;
   // void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;

    //using VocalFilter = VocalFilter<WidgetComposite>;
    VocalFilter<WidgetComposite> vocalFilter;
private:
    typedef float T;
};



VocalFilterModule::VocalFilterModule() : Module(vocalFilter.NUM_PARAMS, vocalFilter.NUM_INPUTS, vocalFilter.NUM_OUTPUTS, vocalFilter.NUM_LIGHTS),
    vocalFilter(this)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    vocalFilter.init();
}


void VocalFilterModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    vocalFilter.setSampleRate(rate);
}

void VocalFilterModule::step()
{
    vocalFilter.step();
}

////////////////////
// module widget
////////////////////


struct VocalFilterWidget : ModuleWidget
{
    VocalFilterWidget(VocalFilterModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
VocalFilterWidget::VocalFilterWidget(VocalFilterModule *module) : ModuleWidget(module)
{
    box.size = Vec(9 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }
}


// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelVocalFilterModule = Model::create<VocalFilterModule, VocalFilterWidget>("Squinky Labs",
    "squinkylabs-vocalfilter",
    "Vocal Filter", EFFECT_TAG, FILTER_TAG);

