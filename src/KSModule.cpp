
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#include "KSComposite.h"


/**
 */
struct KSModule : Module
{
public:
    KSModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    KSComposite<WidgetComposite> composite;
private:

};

void KSModule::onSampleRateChange()
{
}

KSModule::KSModule()
    : Module(composite.NUM_PARAMS,
    composite.NUM_INPUTS,
    composite.NUM_OUTPUTS,
    composite.NUM_LIGHTS),
    composite(this)
{
    onSampleRateChange();
    composite.init();
}

void KSModule::step()
{
    composite.step();
}

////////////////////
// module widget
////////////////////

struct compositeWidget : ModuleWidget
{
    compositeWidget(KSModule *);

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
compositeWidget::compositeWidget(KSModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }


    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


Model *modelKSModule = Model::create<KSModule,
    compositeWidget>("Squinky Labs",
    "squinkylabs-ks",
    "kitchen sink", RANDOM_TAG);

