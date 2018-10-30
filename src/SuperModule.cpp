#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "SQWidgets.h"

#include "Super.h"

/**
 */
struct SuperModule : Module
{
public:
    SuperModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Super<WidgetComposite> super;
private:

};

void SuperModule::onSampleRateChange()
{
}

SuperModule::SuperModule()
    : Module(super.NUM_PARAMS,
    super.NUM_INPUTS,
    super.NUM_OUTPUTS,
    super.NUM_LIGHTS),
    super(this)
{
    onSampleRateChange();
    super.init();
}

void SuperModule::step()
{
    super.step();
}

////////////////////
// module widget
////////////////////

struct superWidget : ModuleWidget
{
    superWidget(SuperModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void addDebug(SuperModule*);
};

void superWidget::addDebug(SuperModule*)
{
    addInput(Port::create<PJ301MPort>(
        Vec(100, 10), Port::INPUT, module, Super<WidgetComposite>::DEBUG_INPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(100, 40), Port::OUTPUT, module, Super<WidgetComposite>::DEBUG_OUTPUT));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
superWidget::superWidget(SuperModule *module) : ModuleWidget(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/super_panel.svg")));
        addChild(panel);
    }

    const float col1 = 40;
    const float col2 = 80;
    const float col3 = 120;

    addDebug(module);

    addOutput(Port::create<PJ301MPort>(
        Vec(60, 330), Port::OUTPUT, module, Super<WidgetComposite>::MAIN_OUTPUT));
    addLabel(
        Vec(60, 310), "out");

    addInput(Port::create<PJ301MPort>(
        Vec(6, 330), Port::INPUT, module, Super<WidgetComposite>::CV_INPUT));
    addLabel(
        Vec(2, 310), "V/8");

    addInput(Port::create<PJ301MPort>(
        Vec(34, 330), Port::INPUT, module, Super<WidgetComposite>::TRIGGER_INPUT));
    addLabel(
        Vec(34, 310), "Trig");

    float labelOffset = -40;
    // Octave
    const float octaveY = 60;
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col1, octaveY), module, Super<WidgetComposite>::OCTAVE_PARAM, -5, 4, 0));
    addLabel(
        Vec(10, octaveY + labelOffset), "Oct");

    // Semi
    const float semiY = 120;
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col1, semiY), module, Super<WidgetComposite>::SEMI_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, semiY + labelOffset), "Semi");

    // Fine
    const float fineY = 180;
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col1, fineY), module, Super<WidgetComposite>::FINE_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, fineY + labelOffset), "Fine");


    // Detune
    const float detuneY = 240;
    labelOffset += 5;
    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, detuneY), module, Super<WidgetComposite>::DETUNE_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, detuneY + labelOffset), "Detune");
    addParam(createParamCentered<Trimpot>(
        Vec(col2, detuneY), module, Super<WidgetComposite>::DETUNE_TRIM_PARAM, -1, 1, 0));
    addInput(createInputCentered<PJ301MPort>(
        Vec(col3, detuneY), module, Super<WidgetComposite>::DETUNE_INPUT));

    // Waveform Mix
    const float mixY = 290;
    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, mixY), module, Super<WidgetComposite>::MIX_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, mixY + labelOffset), "Mix");
    addParam(createParamCentered<Trimpot>(
        Vec(col2, mixY), module, Super<WidgetComposite>::MIX_TRIM_PARAM, -1, 1, 0));
    addInput(createInputCentered<PJ301MPort>(
        Vec(col3, mixY), module, Super<WidgetComposite>::MIX_INPUT));


    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


Model *modelSuperModule = Model::create<SuperModule,
    superWidget>("Squinky Labs",
    "squinkylabs-super",
    "-- super --", RANDOM_TAG);

