
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


struct ColorDisplay : OpaqueWidget {
    ColoredNoiseModule *module;

    void draw(NVGcontext *vg) override 
    {
        const float slope = module->noiseSource.getSlope();

        float red = (slope > 0) ? slope * 25 : 0;
        float blue = (slope < 0) ? slope * -25 : 0;
        // draw some squares for fun
       // nvgScale(vg, 2, 2);
        nvgFillColor(vg, nvgRGBA(red, 0x00, blue, 0xff));
        nvgBeginPath(vg);
        // todo: pass in ctor
        nvgRect(vg, 0, 0, 6 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
		nvgFill(vg);

    }


};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
ColoredNoiseWidget::ColoredNoiseWidget(ColoredNoiseModule *module) : ModuleWidget(module)
{
 
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    #if 1
	{
		ColorDisplay *display = new ColorDisplay();
		display->module = module;
		display->box.pos = Vec( 0, 0);
		display->box.size = Vec(6 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
		addChild(display);
        display->module = module;
	}
 #else

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

#endif
    
    Label * label = new Label();
    label->box.pos = Vec(23, 24);
    label->text = "Noise";
    label->color = COLOR_BLACK;
    addChild(label);

    addOutput(Port::create<PJ301MPort>(
        Vec(20, 200),
        Port::OUTPUT,
        module,
        module->noiseSource.AUDIO_OUTPUT));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(
        Vec(28, 100), module, module->noiseSource.SLOPE_PARAM, -8.0, 8.0, 0.0));



}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelColoredNoiseModule = Model::create<ColoredNoiseModule, ColoredNoiseWidget>(
    "Squinky Labs",
    "squinkylabs-coloredNoise",
    "Colored Noise", EFFECT_TAG, FILTER_TAG);
