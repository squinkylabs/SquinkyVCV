
#include "Squinky.hpp"

#include "WidgetComposite.h"
#include "VocalAnimator.h"


/**
 * Implementation class for VocalWidget
 */
struct VocalModule : Module
{
#if 0
    enum ParamIds
    {
        PITCH_PARAM,      // the big pitch knob
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };
#endif

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

#if 0
    SawOscillatorParams<T> lfoParams;
    SawOscillatorState<T> lfoState;
    T reciprocolSampleRate;
#endif
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
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/booty_panel.svg")));
        addChild(panel);
    }

    addOutput(Port::create<PJ301MPort>(Vec(45, 100), Port::OUTPUT, module, VocalModule::Animator::MAIN_OUTPUT));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelVocalModule = Model::create<VocalModule, VocalWidget>("Squinky Labs",
    "squinkylabs-vocalanimator",
    "Vocal Animator", EFFECT_TAG, FILTER_TAG);

