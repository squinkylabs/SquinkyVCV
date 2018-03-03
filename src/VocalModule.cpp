
#include "Squinky.hpp"

#include "WidgetComposite.h"
#include "VocalAnimator.h"


/**
 * Implementation class for VocalWidget
 */
struct VocalModule : Module
{

    VocalModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
   // json_t *toJson() override;
   // void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;

    using Animator = VocalAnimator<WidgetComposite>;
    Animator animator;
private:
    typedef float T;
};



VocalModule::VocalModule() : Module(animator.NUM_PARAMS, animator.NUM_INPUTS, animator.NUM_OUTPUTS, animator.NUM_LIGHTS),
    animator(this)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    animator.init();
}

void VocalModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    animator.setSampleRate(rate);
}

void VocalModule::step()
{
    animator.step();
}

////////////////////
// module widget
////////////////////


struct VocalWidget : ModuleWidget
{
    VocalWidget(VocalModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
VocalWidget::VocalWidget(VocalModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }
    // Let's add some LED's for fun
    const float ledY = 24;
    const float ledSpace = 30;
    const float ledX = 10;
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(ledX, ledY), module, module->animator.LFO0_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(ledX + ledSpace, ledY), module, module->animator.LFO1_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(ledX + ledSpace * 2, ledY), module, module->animator.LFO2_LIGHT));
    
    /*
      LFO_RATE_PARAM,
        LFO_SPREAD_PARAM,
        FILTER_Q_PARAM,
        FILTER_FC_PARAM,
        FILTER_MOD_DEPTH_PARAM,
        */
    
    const float knobX = 26;
    const float knoby = 50;
    const float space = 50;
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knoby), module, module->animator.LFO_RATE_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knoby + 1 * space), module, module->animator.LFO_SPREAD_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knoby + 2 * space), module, module->animator.FILTER_Q_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knoby + 3 * space), module, module->animator.FILTER_FC_PARAM, -5.0, 5.0, 0.0));
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knoby + 4 * space), module, module->animator.FILTER_MOD_DEPTH_PARAM, -5.0, 5.0, 0.0));

     const float row3 = 317.5;

    // I.O on row 3
    const float inputX = 9.5;
    const float outputX = 55.5;
 
    addInput(Port::create<PJ301MPort>(Vec(inputX, row3), Port::INPUT, module, VocalModule::Animator::AUDIO_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(outputX, row3), Port::OUTPUT, module, VocalModule::Animator::AUDIO_OUTPUT));
    
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
Model *modelVocalModule = Model::create<VocalModule, VocalWidget>("Squinky Labs",
    "squinkylabs-vocalanimator",
    "Vocal Animator", EFFECT_TAG, FILTER_TAG);

