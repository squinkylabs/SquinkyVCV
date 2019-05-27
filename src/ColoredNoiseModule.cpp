
#include "Squinky.hpp"

#ifdef _COLORS
#include "WidgetComposite.h"
#include "ColoredNoise.h"
#include "NoiseDrawer.h"
#include "ctrl/SqMenuItem.h"

using Comp = ColoredNoise<WidgetComposite>;
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

    std::shared_ptr<Comp> noiseSource;
private:
    typedef float T;
};

#ifdef __V1
ColoredNoiseModule::ColoredNoiseModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    noiseSource = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
#else
ColoredNoiseModule::ColoredNoiseModule()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS),
    noiseSource(std::make_shared<Comp>(this))
{
#endif
    onSampleRateChange();
    noiseSource->init();
}

void ColoredNoiseModule::onSampleRateChange()
{
    T rate = SqHelper::engineGetSampleRate();
    noiseSource->setSampleRate(rate);
}

void ColoredNoiseModule::step()
{
    noiseSource->step();
}
 
////////////////////
// module widget
////////////////////

struct ColoredNoiseWidget : ModuleWidget
{
    ColoredNoiseWidget(ColoredNoiseModule *);
    Label * slopeLabel;
    Label * signLabel;

    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/colors.md");
};

// The colors of noise (UI colors)
static const unsigned char red[3] = {0xff, 0x04, 0x14};
static const unsigned char pink[3] = {0xff, 0x3a, 0x6d};
static const unsigned char white[3] = {0xff, 0xff, 0xff};
static const unsigned char blue[3] = {0x54, 0x43, 0xc1};
static const unsigned char violet[3] = {0x9d, 0x3c, 0xe6};

// 0 <= x <= 1
static float interp(float x, int x0, int x1)
{
    return x1 * x + x0 * (1 - x);
}

// 0 <= x <= 3
static void interp(unsigned char * out, float x, const unsigned char* y0, const unsigned char* y1)
{
    x = x * 1.0 / 3.0;    // 0..1
    out[0] = interp(x, y0[0], y1[0]);
    out[1] = interp(x, y0[1], y1[1]);
    out[2] = interp(x, y0[2], y1[2]);
}

static void copyColor(unsigned char * out, const unsigned char* in)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

static void getColor(unsigned char * out, float x)
{
    if (x < -6) {
        copyColor(out, red);
    } else if (x >= 6) {
        copyColor(out, violet);
    } else {
        if (x < -3) {
            interp(out, x + 6, red, pink);
        } else if (x < 0) {
            interp(out, x + 3, pink, white);
        } else if (x < 3) {
            interp(out, x + 0, white, blue);
        } else if (x < 6) {
            interp(out, x - 3, blue, violet);
        } else {
            copyColor(out, white);
        }
    }
}

// the draw size of the colored noise display.
const int colorWidth = 85;
const int colorHeight = 180;
const int colorX = 10;
const int colorY = 170;

struct ColorDisplay : TransparentWidget
{
    ColoredNoiseModule *module;
    ColorDisplay(Label *slopeLabel, Label *signLabel)
        : _slopeLabel(slopeLabel),
        _signLabel(signLabel)
    {
    }

    Label* _slopeLabel;
    Label* _signLabel;
    std::unique_ptr<NoiseDrawer> _noiseDrawer;

    void draw(NVGcontext *vg) override
    {
        // First draw the solid fill
        float slope = 0;
        if (module) {
            slope = module->noiseSource->getSlope();
        }
        unsigned char color[3];
        getColor(color, slope);
        nvgFillColor(vg, nvgRGBA(color[0], color[1], color[2], 0xff));

        nvgBeginPath(vg);

        nvgRect(vg, colorX, colorY, 6 * colorWidth, colorHeight);
        nvgFill(vg);

        // then the noise
        if (!_noiseDrawer) {
            // TODO: this 100x100 was a mistake, but now we like the
            // slight stretching. look into this some more to try and
            // improve the looks later.
            _noiseDrawer.reset(new NoiseDrawer(vg, 100, 100));
        }
        _noiseDrawer->draw(vg, colorX, colorY, colorWidth, colorHeight);


        // update the slope display in the UI
        const bool slopeSign = slope >= 0;
        const float slopeAbs = std::abs(slope);
        std::stringstream s;
        s.precision(1);
        s.setf(std::ios::fixed, std::ios::floatfield);

        s << slopeAbs << " db/oct";
        _slopeLabel->text = s.str();

        const char * mini = "\u2005-";
        _signLabel->text = slopeSign ? "+" : mini;
    }
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
ColoredNoiseWidget::ColoredNoiseWidget(ColoredNoiseModule* module)
{   
    setModule(module);
#else
ColoredNoiseWidget::ColoredNoiseWidget(ColoredNoiseModule *module) : ModuleWidget(module)
{
#endif
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    // save so we can update later.
    slopeLabel = new Label();
    signLabel = new Label();

    // add the color display
    {
        ColorDisplay *display = new ColorDisplay(slopeLabel, signLabel);
        display->module = module;
        display->box.pos = Vec(0, 0);
        display->box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        addChild(display);
        display->module = module;
    }


    // Add the background panel
#ifndef __V1
   SqHelper::setPanel(this, "res/colors_panel.svg");
#else
    {
        // use the code from ModuleWidget::setPanel, but don't jam to bottom
        auto svg = SqHelper::loadSvg("res/colors_panel.svg");
        SvgPanel *svgPanel = new SvgPanel;
        svgPanel->setBackground(svg);
        panel = svgPanel;
        addChild(panel);

        // Set ModuleWidget size based on panel
        box.size.x = std::round(panel->box.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
    }
#endif

    addOutput(createOutput<PJ301MPort>(
        Vec(32, 310),
        module,
        Comp::AUDIO_OUTPUT));
    Label* label = new Label();
    label->box.pos = Vec(24.2, 294);
    label->text = "OUT";
    label->color = SqHelper::COLOR_WHITE;
    addChild(label);

    addParam(SqHelper::createParam<Rogan2PSBlue>(
        icomp,
        Vec(22, 80),
        module,
        Comp::SLOPE_PARAM));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(58, 46),
        module, 
        Comp::SLOPE_TRIM));

    addInput(createInput<PJ301MPort>(
        Vec(14, 42),
        module,
        Comp::SLOPE_CV));

    // Create the labels for slope. They will get
    // text content later.
    const float labelY = 146;
    slopeLabel->box.pos = Vec(12, labelY);
    slopeLabel->text = "";
    slopeLabel->color = SqHelper::COLOR_BLACK;
    addChild(slopeLabel);
    signLabel->box.pos = Vec(2, labelY);
    signLabel->text = "";
    signLabel->color = SqHelper::COLOR_BLACK;
    addChild(signLabel);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

#ifdef __V1
Model *modelColoredNoiseModule = createModel<ColoredNoiseModule, ColoredNoiseWidget>(
    "squinkylabs-coloredNoise");
#else
Model *modelColoredNoiseModule = Model::create<ColoredNoiseModule, ColoredNoiseWidget>(
    "Squinky Labs",
    "squinkylabs-coloredNoise",
    "Colors: Colored Noise", NOISE_TAG);
#endif
#endif



