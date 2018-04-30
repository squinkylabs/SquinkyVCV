
#include "Squinky.hpp"

#include "WidgetComposite.h"
#include "ColoredNoise.h"

/**
 * Implementation class for VocalWidget
 */
struct ColoredNoiseModule : Module
{
    ColoredNoiseModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    // TODO: real composite
    ColoredNoise<WidgetComposite> noiseSource;
private:
    typedef float T;
};

ColoredNoiseModule::ColoredNoiseModule() : Module(noiseSource.NUM_PARAMS,
                     noiseSource.NUM_INPUTS, 
                     noiseSource.NUM_OUTPUTS, 
                     noiseSource.NUM_LIGHTS),
                     noiseSource(this)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    noiseSource.init();
}

void ColoredNoiseModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    noiseSource.setSampleRate(rate);
}

void ColoredNoiseModule::step()
{
    noiseSource.step();
}

////////////////////
// module widget
////////////////////

struct ColoredNoiseWidget : ModuleWidget
{
    ColoredNoiseWidget(ColoredNoiseModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
ColoredNoiseWidget::ColoredNoiseWidget(ColoredNoiseModule *module) : ModuleWidget(module)
{
 
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    
    Label * label = new Label();
    label->box.pos = Vec(23, 24);
    label->text = "Noise";
    label->color = COLOR_BLACK;
    addChild(label);
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelColoredNoiseModule = Model::create<ColoredNoiseModule, ColoredNoiseWidget>(
    "Squinky Labs",
    "squinkylabs-coloredNoise",
    "Colored Noise", EFFECT_TAG, FILTER_TAG);
