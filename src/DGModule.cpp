
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "daveguide.h"

/**
 */
struct DGModule : Module
{
public:
    DGModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

    Daveguide<WidgetComposite> dave;
private:
};

DGModule::DGModule()
    : Module(dave.NUM_PARAMS,
    dave.NUM_INPUTS,
    dave.NUM_OUTPUTS,
    dave.NUM_LIGHTS),
    dave(this)
{
}

void DGModule::step()
{
    dave.step();
}

////////////////////
// module widget
////////////////////

struct DGWidget : ModuleWidget
{
    DGWidget(DGModule *);

    /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }


private:
    DGModule* const module;
};




/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
DGWidget::DGWidget(DGModule *module) :
    ModuleWidget(module),
    module(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

     addLabel(Vec(35, 20), "Daveguide");

    addInput(createInputCentered<PJ301MPort>(
            Vec(40,340),
            module,
            Daveguide<WidgetComposite>::INPUT_AUDIO));

    addOutput(createOutputCentered<PJ301MPort>(
            Vec(120,340),
            module,
            Daveguide<WidgetComposite>::OUTPUT_AUDIO));


    const float labelDeltaY = 25;
    const float gainX = 40;
    const float offsetX = 120;
    const float labelDeltaX = -20;
    const float y = 170;

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(gainX, y),
        module, Daveguide<WidgetComposite>::PARAM_DELAY, -5, 5, 0));
    addLabel(Vec(gainX+labelDeltaX, y + labelDeltaY), "delay");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(offsetX, y),
        module, Daveguide<WidgetComposite>::PARAM_FEEDBACK, -5, 5, 0));
    addLabel(Vec(offsetX+labelDeltaX,  y + labelDeltaY), "offset");



 
    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 
}

Model *modelDGModule = Model::create<DGModule,
    DGWidget>("Squinky Labs",
    "squinkylabs-dvg",
    "dg", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);

