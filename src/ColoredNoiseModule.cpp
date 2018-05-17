
#ifdef _EXP
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
    Label * slopeLabel;
};


// F44336
//static const unsigned char red[3] = {0xf4, 0x43, 0x36 };
static const unsigned char red[3] = {0xff, 0x04, 0x14 };
// EC407A
//static const unsigned char pink[3] = {0xec, 0x40, 0x7a };
static const unsigned char pink[3] = {0xff, 0x3a, 0x6d };

static const unsigned char white[3] = {0xe0, 0xe0, 0xe0 };

// #3F51B5
//static const unsigned char blue[3] = {0x3f, 0x51, 0xb5 };
static const unsigned char blue[3] = {0x54, 0x43, 0xc1 };

// 9C27B0
//static const unsigned char violet[3] = {0x9c, 0x27, 0xb0 };
static const unsigned char violet[3] = {0x9d, 0x3c, 0xe6 };

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
    ColorDisplay(Label *l) : theLabel(l) {}

    Label* theLabel=0;
    void draw(NVGcontext *vg) override 
    {
        const float slope = module->noiseSource.getSlope();
        unsigned char color[3];
        getColor(color, slope);
        nvgFillColor(vg, nvgRGBA(color[0], color[1], color[2], 0xff));

        nvgBeginPath(vg);
        // todo: pass in ctor
        nvgRect(vg, 0, 0, 6 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
		nvgFill(vg);

        std::stringstream s;
        s.precision(1);
        s.setf( std::ios::fixed, std::ios::floatfield );
        s << slope << " db/oct";
        theLabel->text = s.str();

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

    slopeLabel = new Label();
	{
		ColorDisplay *display = new ColorDisplay(slopeLabel);
		display->module = module;
		display->box.pos = Vec( 0, 0);
		display->box.size = Vec(6 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
		addChild(display);
        display->module = module;
	}

    
    Label * label = new Label();
    label->box.pos = Vec(23, 24);
    label->text = "Noise";
    label->color = COLOR_BLACK;
    addChild(label);

    addOutput(Port::create<PJ301MPort>(
        Vec(20, 300),
        Port::OUTPUT,
        module,
        module->noiseSource.AUDIO_OUTPUT));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(
        Vec(28, 100), module, module->noiseSource.SLOPE_PARAM, -5.0, 5.0, 0.0));

    addParam(ParamWidget::create<Trimpot>(
        Vec(28, 140),
        module, module->noiseSource.SLOPE_TRIM, -1.0, 1.0, 1.0));

    addInput(Port::create<PJ301MPort>(
        Vec(20, 160),
        Port::INPUT,
        module,
        module->noiseSource.SLOPE_CV));

    
    slopeLabel->box.pos = Vec(6, 50);
    slopeLabel->text = "slope";
    slopeLabel->color = COLOR_BLACK;
    addChild(slopeLabel);

}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelColoredNoiseModule = Model::create<ColoredNoiseModule, ColoredNoiseWidget>(
    "Squinky Labs",
    "squinkylabs-coloredNoise",
    "Colored Noise", EFFECT_TAG, FILTER_TAG);

#endif
