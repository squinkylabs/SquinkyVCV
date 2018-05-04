
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


// F44336
static const unsigned char red[3] = {0xf4, 0x43, 0x36 };
// EC407A
static const unsigned char pink[3] = {0xec, 0x40, 0x7a };

static const unsigned char white[3] = {0xe0, 0xe0, 0xe0 };
// #3F51B5
static const unsigned char blue[3] = {0x3f, 0x51, 0xb5 };
// 9C27B0
static const unsigned char violet[3] = {0x9c, 0x27, 0xb0 };

// 0 <= x <= 1
static float interp(float x, int x0, int x1) 
{
    return x1 * x + x0 * (1 - x);
}

// 0 <= x <= 3
static void interp(unsigned char * out, float x, const unsigned char* y0, const unsigned char* y1)
{
    x = x * 1.0/3.0;    // 0..1
    out[0] = interp (x, y0[0], y1[0]);
    out[1] = interp (x, y0[1], y1[1]);
    out[2] = interp (x, y0[2], y1[2]);
}

static void copyColor(unsigned char * out, const unsigned char* in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

static void getColor(unsigned char * out,  float x)
{
    if (x < -6) {
       copyColor(out, red);
    } else if (x >= 6) {
        copyColor(out, violet);
    } else {
        if (x < -3) {
            interp(out, x + 6, red, pink);
        } else if ( x < 0) {
            interp(out, x + 3, pink, white);
        } else if ( x < 3) {
            interp(out, x + 0, white, blue);
        } else if ( x < 6) {
            interp(out, x - 3, blue, violet);
        } else {
            copyColor(out, white);
        }
    }

}


struct ColorDisplay : OpaqueWidget {
    ColoredNoiseModule *module;

    void draw(NVGcontext *vg) override 
    {
        const float slope = module->noiseSource.getSlope();
#if 1
        unsigned char color[3];
        getColor(color, slope);
        nvgFillColor(vg, nvgRGBA(color[0], color[1], color[2], 0xff));

#else
        float red = (slope > 0) ? slope * 25 : 0;
        float blue = (slope < 0) ? slope * -25 : 0;
        // draw some squares for fun
       // nvgScale(vg, 2, 2);
        nvgFillColor(vg, nvgRGBA(red, 0x00, blue, 0xff));
#endif



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
