
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "Tremolo.h"

/**
 */
struct TremoloModule : Module
{
public:
    TremoloModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;
    Tremolo<WidgetComposite> tremolo;
private:
};

void TremoloModule::onSampleRateChange()
{
    float rate = engineGetSampleRate();
    tremolo.setSampleRate(rate);
}

TremoloModule::TremoloModule()
    : Module(tremolo.NUM_PARAMS,
    tremolo.NUM_INPUTS,
    tremolo.NUM_OUTPUTS,
    tremolo.NUM_LIGHTS),
    tremolo(this)
{
    onSampleRateChange();
    tremolo.init();
}

void TremoloModule::step()
{
    tremolo.step();
}

////////////////////
// module widget
////////////////////

struct TremoloWidget : ModuleWidget
{
    TremoloWidget(TremoloModule *);

    void addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
    }
    void addClockSection(TremoloModule *module);
    void addIOSection(TremoloModule *module);
    void addMainSection(TremoloModule *module);
};


void TremoloWidget::addClockSection(TremoloModule *module)
{
    const float y = 40;        // global offset for clock block
    const float labelY = y + 36;

    addInput(Port::create<PJ301MPort>(Vec(5, y + 7), Port::INPUT, module, module->tremolo.CLOCK_INPUT));
    addLabel(Vec(5, labelY), "ckin");

    addParam(ParamWidget::create<RoundBlackKnob>(
        Vec(110, y), module, module->tremolo.LFO_RATE_PARAM, -5.0, 5.0, 0.0));
    addLabel(Vec(102, labelY), "Rate");

    const float cmy = y;
    const float cmx = 60;
    addParam(ParamWidget::create<RoundBlackSnapKnob>(
        Vec(cmx, cmy), module, module->tremolo.CLOCK_MULT_PARAM, 0.0f, 4.0f, 0.0f));
    addLabel(Vec(cmx - 10, labelY), "Clock");
    addLabel(Vec(cmx - 17, cmy + 20), "x1");
    addLabel(Vec(cmx + 15, cmy + 20), "int");
    addLabel(Vec(cmx - 23, cmy + 0), "x2");
    addLabel(Vec(cmx + 22, cmy + 0), "x4");
    addLabel(Vec(cmx, cmy - 12), "x3");
}

void TremoloWidget::addIOSection(TremoloModule *module)
{
    const float rowIO = 316;
    const float label = rowIO - 20;
    const float deltaX = 35;
    const float x = 10;

    addInput(Port::create<PJ301MPort>(Vec(x, rowIO), Port::INPUT, module, module->tremolo.AUDIO_INPUT));
    addLabel(Vec(8, label), "in");

    addOutput(Port::create<PJ301MPort>(Vec(x + deltaX, rowIO), Port::OUTPUT, module, module->tremolo.AUDIO_OUTPUT));
    addLabel(Vec(x + deltaX - 6, label), "out");

    addOutput(Port::create<PJ301MPort>(Vec(x + 2 * deltaX, rowIO), Port::OUTPUT, module, module->tremolo.SAW_OUTPUT));
    addLabel(Vec(x + 2 * deltaX - 6, label), "saw");

    addOutput(Port::create<PJ301MPort>(Vec(x + 3 * deltaX, rowIO), Port::OUTPUT, module, module->tremolo.LFO_OUTPUT));
    addLabel(Vec(x + 3 * deltaX - 6, label), "lfo");
}

void TremoloWidget::addMainSection(TremoloModule *module)
{
    const float knobX = 64;
    const float knobY = 100;
    const float knobDy = 50;
    const float labelX = 100;
    const float labelY = knobY;
    const float trimX = 40;
    const float trimY = knobY + 10;
    const float inY = knobY + 6;
    const float inX = 8;

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX, knobY + 0 * knobDy), module, module->tremolo.LFO_SHAPE_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX, trimY + 0 * knobDy), module, module->tremolo.LFO_SHAPE_TRIM_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inX, inY + 0 * knobDy), Port::INPUT, module, module->tremolo.LFO_SHAPE_INPUT));
    addLabel(
        Vec(labelX, labelY + 0 * knobDy), "Shape");

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX, knobY + 1 * knobDy), module, module->tremolo.LFO_SKEW_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX, trimY + 1 * knobDy), module, module->tremolo.LFO_SKEW_TRIM_PARAM, -1.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inX, labelY + 1 * knobDy + 6), Port::INPUT, module, module->tremolo.LFO_SKEW_INPUT));
    addLabel(
        Vec(labelX, labelY + 1 * knobDy), "Skew");

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX, knobY + 2 * knobDy), module, module->tremolo.LFO_PHASE_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX, trimY + 2 * knobDy), module, module->tremolo.LFO_PHASE_TRIM_PARAM, -1.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inX, labelY + 2 * knobDy + 6), Port::INPUT, module, module->tremolo.LFO_PHASE_INPUT));
    addLabel(
        Vec(labelX, labelY + 2 * knobDy), "Phase");

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX, knobY + 3 * knobDy), module, module->tremolo.MOD_DEPTH_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(
        Vec(trimX, trimY + 3 * knobDy), module, module->tremolo.MOD_DEPTH_TRIM_PARAM, -1.0, 1.0, 1.0));
    addInput(Port::create<PJ301MPort>(
        Vec(inX, labelY + 3 * knobDy + 6), Port::INPUT, module, module->tremolo.MOD_DEPTH_INPUT));
    addLabel(
        Vec(labelX, labelY + 3 * knobDy), "Depth");
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
TremoloWidget::TremoloWidget(TremoloModule *module) : ModuleWidget(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/trem_panel.svg")));
        addChild(panel);
    }

    addClockSection(module);
    addMainSection(module);
    addIOSection(module);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelTremoloModule = Model::create<TremoloModule,
    TremoloWidget>("Squinky Labs",
    "squinkylabs-tremolo",
    "Tremolo", EFFECT_TAG, LFO_TAG);

