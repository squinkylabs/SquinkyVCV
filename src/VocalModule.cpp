
#ifdef _EXP // experimental module

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
    box.size = Vec(9 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }
    /**
     *  LEDs and LFO outputs
     */
    const float ledY = 28;
    const float ledSpace = 43;
    const float ledX = 7;
    const float lfoOutX = 20;
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(ledX, ledY), module, module->animator.LFO0_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(ledX + ledSpace, ledY), module, module->animator.LFO1_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(ledX + 2*ledSpace, ledY), module, module->animator.LFO2_LIGHT));
    
    addOutput(Port::create<PJ301MPort>(Vec(lfoOutX, ledY), Port::OUTPUT, module, VocalModule::Animator::LFO0_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(lfoOutX+ledSpace, ledY), Port::OUTPUT, module, VocalModule::Animator::LFO1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(lfoOutX+2*ledSpace, ledY), Port::OUTPUT, module, VocalModule::Animator::LFO2_OUTPUT));

    /**
     * Parameters and CV
     */ 
    const float knobX = 75;
    const float trimX = 46;
    const float inputX = 8;
    float knobY = 60;
    const float space = 46;
    const float labelX = 0;
    const float labelOffset = 0;
    const float inputYOffset = 16;
    const float trimYOffset = 18;
    
    Label *label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Rate";
    label->color = COLOR_BLACK;
    addChild(label);  
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->animator.LFO_RATE_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, VocalModule::Animator::LFO_RATE_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->animator.LFO_RATE_TRIM_PARAM, -1.0, 1.0, 1.0));

    knobY += space;
    label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Mod Spread";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->animator.LFO_SPREAD_PARAM, -5.0, 5.0, 0.0));
    const float matrixY = knobY;

    knobY += space;
    label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Q";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->animator.FILTER_Q_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, VocalModule::Animator::FILTER_Q_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->animator.FILTER_Q_TRIM_PARAM, -1.0, 1.0, 1.0));
   
    knobY += space;
    label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Fc filter";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->animator.FILTER_FC_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, VocalModule::Animator::FILTER_FC_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->animator.FILTER_FC_TRIM_PARAM, -1.0, 1.0, 1.0));
      
    knobY += space;
    label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Mod Depth";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->animator.FILTER_MOD_DEPTH_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, VocalModule::Animator::FILTER_MOD_DEPTH_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->animator.FILTER_MOD_DEPTH_TRIM_PARAM, -1.0, 1.0, 1.0));
  
    const float row3 = 317.5;

    // I.O on row 3
    const float AudioInputX = 10.0;
    const float outputX = 57.0;
 
    addInput(Port::create<PJ301MPort>(Vec(AudioInputX, row3), Port::INPUT, module, VocalModule::Animator::AUDIO_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(outputX, row3), Port::OUTPUT, module, VocalModule::Animator::AUDIO_OUTPUT));
    
    label = new Label();
    label->box.pos = Vec(AudioInputX, row3 - 20);
    label->text = "In";
    label->color = COLOR_BLACK;
    addChild(label);  
    label = new Label();
    label->box.pos = Vec(outputX, row3 - 20);
    label->text = "Out";
    label->color = COLOR_BLACK;
    addChild(label);  
    
    /*******************************************
     *  temp test switches
     */
    const float switchX = 120;      
    // 2 pos bass_exp
    addParam(ParamWidget::create<CKSS>( Vec(switchX, 205), module, module->animator.BASS_EXP_PARAM, 0.0f, 1.0f, 0.0f));     
    label = new Label();
    label->box.pos = Vec(switchX -8, 225);
    label->text = "B";
    label->color = COLOR_BLACK;;
    addChild(label); 
   
    // 3 pos track_exp
    addParam(ParamWidget::create<CKSSThree>( Vec(switchX,255), module, module->animator.TRACK_EXP_PARAM, 0.0f, 2.0f, 0.0f));
    label = new Label();
    label->box.pos = Vec(switchX -12, 284);
    label->text = "Scl";
    label->color = COLOR_BLACK;
    addChild(label);  

    // 3 pos LFO matrix
    addParam(ParamWidget::create<CKSSThree>( Vec(switchX,matrixY), module, module->animator.LFO_MIX_PARAM, 0.0f, 2.0f, 0.0f));
    label = new Label();
    label->box.pos = Vec(switchX -15, matrixY+28);
    label->text = "Mtx";
    label->color = COLOR_BLACK;
    addChild(label);  

    /*************************************************
     *  screws
     */
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

#endif