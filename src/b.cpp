
#include "Squinky.hpp"
#include "WidgetComposite.h"

/**
 * Implementation class for BootyModule
 */
struct bModule : Module
{
	enum ParamIds {
		THREAD_BOOST_PARAM,
		NUM_PARAMS
	};
	enum InputIds {

		NUM_INPUTS
	};
	enum OutputIds {

		NUM_OUTPUTS
	};
	enum LightIds {
        NORMAL_LIGHT,
        BOOST_LIGHT,
        REALTIME_LIGHT,
        ERROR_LIGHT,
		NUM_LIGHTS
	};

    bModule();

    /**
     * Overrides of Module functions
     */
    void step() override;

private:
    typedef float T;

};

//extern float values[];
//extern const char* ranges[];

bModule::bModule() : Module(NUM_PARAMS,NUM_INPUTS,NUM_OUTPUTS,NUM_LIGHTS)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    //shifter.init();
}


void bModule::step()
{
 
}

////////////////////
// module widget
////////////////////

struct bWidget : ModuleWidget
{
    bWidget(bModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
bWidget::bWidget(bModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    addParam(ParamWidget::create<NKK>(
        Vec(30, 140), module, bModule::THREAD_BOOST_PARAM, 0.0f, 2.0f, 0.0f));

    const int ledX = 10;
    const int labelX = 16;
    const int ledY = 200;
    const int labelY = ledY - 5;
    const int deltaY = 30;
    Label* label;
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY), module, bModule::NORMAL_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY);
    label->text = "Normal";
    label->color = COLOR_BLACK;
    addChild(label);

    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY+deltaY), module, bModule::BOOST_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY+deltaY);
    label->text = "Boost";
    label->color = COLOR_BLACK;
    addChild(label);

     addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY+2*deltaY), module, bModule::REALTIME_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY+2*deltaY);
    label->text = "Real-time";
    label->color = COLOR_BLACK;
    addChild(label);

     addChild(ModuleLightWidget::create<MediumLight<RedLight>>(
        Vec(ledX, ledY+3*deltaY), module, bModule::ERROR_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY+3*deltaY);
    label->text = "Normal";
    label->color = COLOR_BLACK;
    addChild(label);


   
    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelBModule = Model::create<bModule, bWidget>("Squinky Labs",
    "squinkylabs-b",
    "b", EFFECT_TAG);

