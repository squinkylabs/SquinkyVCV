
#include "Squinky.hpp"


#include "WidgetComposite.h"
#include "VocalFilter.h"


/**
 * Implementation class for VocalWidget
 */
struct VocalFilterModule : Module
{

    VocalFilterModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
   // json_t *toJson() override;
   // void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;

    //using VocalFilter = VocalFilter<WidgetComposite>;
    VocalFilter<WidgetComposite> vocalFilter;
private:
    typedef float T;
};



VocalFilterModule::VocalFilterModule() : Module(vocalFilter.NUM_PARAMS, vocalFilter.NUM_INPUTS, vocalFilter.NUM_OUTPUTS, vocalFilter.NUM_LIGHTS),
    vocalFilter(this)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    vocalFilter.init();
}


void VocalFilterModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    vocalFilter.setSampleRate(rate);
}

void VocalFilterModule::step()
{
    vocalFilter.step();
}

////////////////////
// module widget
////////////////////


struct VocalFilterWidget : ModuleWidget
{
    VocalFilterWidget(VocalFilterModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
VocalFilterWidget::VocalFilterWidget(VocalFilterModule *module) : ModuleWidget(module)
{
    box.size = Vec(9 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }


    const float knobX = 75;
    const float trimX = 46;
    const float inputX = 8;
    float knobY = 60;
    const float space = 46;
    const float labelX = 0;
    const float labelOffset = 0;
    const float inputYOffset = 16;
    const float trimYOffset = 18;
    const float switchX = 120;  

    // Filter FC
    knobY += space;
    Label *label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Fc filter";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->vocalFilter.FILTER_FC_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, module->vocalFilter.FILTER_FC_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->vocalFilter.FILTER_FC_TRIM_PARAM, -1.0, 1.0, 1.0));
      
    //Vowels
    knobY += space;
    label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Vowel";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->vocalFilter.FILTER_VOWEL_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, module->vocalFilter.FILTER_VOWEL_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->vocalFilter.FILTER_VOWEL_TRIM_PARAM, -1.0, 1.0, 1.0));

   //Q  
    knobY += space;
    label = new Label();
    label->box.pos = Vec(labelX, knobY+labelOffset);
    label->text = "Q";
    label->color = COLOR_BLACK;
    addChild(label);
    addParam(ParamWidget::create<Rogan1PSBlue>(Vec(knobX, knobY), module, module->vocalFilter.FILTER_Q_PARAM, -5.0, 5.0, 0.0));
    addInput(Port::create<PJ301MPort>(Vec(inputX, knobY+inputYOffset), Port::INPUT, module, module->vocalFilter.FILTER_Q_CV_INPUT));
    addParam(ParamWidget::create<Trimpot>(Vec(trimX, knobY+trimYOffset), module, module->vocalFilter.FILTER_Q_TRIM_PARAM, -1.0, 1.0, 1.0));
   

    // 3 pos vocal model
    addParam(ParamWidget::create<CKSSThree>( Vec(switchX,255), module, module->vocalFilter.FILTER_MODEL_SELECT_PARAM, 0.0f, 2.0f, 0.0f));
    label = new Label();
    label->box.pos = Vec(switchX -50, 284);
    label->text = "Model";
    label->color = COLOR_BLACK;
    addChild(label);  

    // I.O on row 3
    const float AudioInputX = 10.0;
    const float outputX = 57.0;
    const float row3 = 317.5;
 
    addInput(Port::create<PJ301MPort>(Vec(AudioInputX, row3), Port::INPUT, module, module->vocalFilter.AUDIO_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(outputX, row3), Port::OUTPUT, module, module->vocalFilter.AUDIO_OUTPUT));
    
}


// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelVocalFilterModule = Model::create<VocalFilterModule, VocalFilterWidget>("Squinky Labs",
    "squinkylabs-vocalfilter",
    "Vocal Filter", EFFECT_TAG, FILTER_TAG);

