
#include "Squinky.hpp"

#include "SawOscillator.h"

#include "WidgetComposite.h"
#if 0
#include "SinOscillator.h"
#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "HilbertFilterDesigner.h"
#endif

/**
 * Implementation class for VocalWidget
 */
struct VocalModule : Module
{
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

    VocalModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
   // json_t *toJson() override;
   // void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;

    //FrequencyShifter<WidgetComposite> shifter;
private:
  typedef float T;
  
  SawOscillatorParams<T> lfoParams;
  SawOscillatorState<T> lfoState;
  T reciprocolSampleRate;
};



VocalModule::VocalModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();

  //  shifter.init();
#if 0
    // Force lazy load of lookup table with this extra call
    
    SinOscillator<T, true>::setFrequency(oscParams, .01);
   
    // Make 128 entry table to do exp x={-5..5} y={2..2000}
    std::function<double(double)> expFunc = AudioMath::makeFunc_Exp(-5, 5, 2, 2000);
    LookupTable<T>::init(exponential, 128, -5, 5, expFunc);
#endif
}

void VocalModule::onSampleRateChange()
{

    T rate = engineGetSampleRate();
    reciprocolSampleRate = 1 / rate;
    SawOscillator<T, false>::setFrequency(lfoParams, reciprocolSampleRate);
    #if 0
    
    HilbertFilterDesigner<T>::design(rate, hilbertFilterParamsSin, hilbertFilterParamsCos);
    #endif
    //shifter.setSampleRate(rate);

}


void VocalModule::step()
{
   outputs[MAIN_OUTPUT].value =  SawOscillator<T, false>::runTri(lfoState, lfoParams);
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
    // BootyModule *module = new BootyModule();
 //	setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/booty_panel.svg")));
        addChild(panel);
    }
    
     addOutput(Port::create<PJ301MPort>(Vec(45, 100), Port::OUTPUT, module, VocalModule::MAIN_OUTPUT));
    
#if 0
    const int leftInputX = 11;
    const int rightInputX = 55;

    const int row0 = 45;
    const int row1 = 102;
    static int row2 = 186;

    // Inputs on Row 0
    addInput(Port::create<PJ301MPort>(Vec(leftInputX, row0), Port::INPUT, module, BootyModule::AUDIO_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(rightInputX, row0), Port::INPUT, module, BootyModule::CV_INPUT));

    // shift Range on row 2
    const float margin = 16;
    float xPos = margin;
    float width = 6 * RACK_GRID_WIDTH - 2 * margin;

    // TODO: why do we need to reach into the module from here? I guess any
    // time UI callbacks need to go bak..
    module->rangeChoice = new RangeChoice(&module->shifter.freqRange, Vec(xPos, row2), width);
    addChild(module->rangeChoice);

    // knob on row 1
    addParam(ParamWidget::create<Rogan3PSBlue>(Vec(18, row1), module, BootyModule::PITCH_PARAM, -5.0, 5.0, 0.0));

    const float row3 = 317.5;

    // Outputs on row 3
    const float leftOutputX = 9.5;
    const float rightOutputX = 55.5;

    addOutput(Port::create<PJ301MPort>(Vec(leftOutputX, row3), Port::OUTPUT, module, BootyModule::SIN_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(rightOutputX, row3), Port::OUTPUT, module, BootyModule::COS_OUTPUT));

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    #endif
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelVocalModule = Model::create<VocalModule, VocalWidget>("Squinky Labs",
    "squinkylabs-vocalanimator",
    "Vocal Animator", EFFECT_TAG, FILTER_TAG);

