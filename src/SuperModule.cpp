
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
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
superWidget::superWidget(SuperModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    addOutput(Port::create<PJ301MPort>(
        Vec(50, 330), Port::OUTPUT, module,  Super<WidgetComposite>::MAIN_OUTPUT));
    addLabel(
        Vec(50 , 300), "out");

    addInput(Port::create<PJ301MPort>(
        Vec(10, 330), Port::INPUT, module,  Super<WidgetComposite>::CV_INPUT));
    addLabel(
        Vec(10 , 300), "V/Oct");

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(10, 30), module, Super<WidgetComposite>::OCTAVE_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, 10), "Oct");

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(10, 95), module, Super<WidgetComposite>::SEMI_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, 75), "Semi");

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(10, 160), module, Super<WidgetComposite>::FINE_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, 140), "Fine");

    addParam(ParamWidget::create<Blue30Knob>(
        Vec(10, 220), module, Super<WidgetComposite>::DETUNE_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, 200), "Detune");

    addParam(ParamWidget::create<Blue30Knob>(
        Vec(10, 270), module, Super<WidgetComposite>::MIX_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, 250), "Mix");

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

