
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "Tremolo.h"

/**
 */
struct TremoloModule : Module
{
public:
    TremoloModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;
   Tremolo<WidgetComposite> tremolo;
private:
 
};

void TremoloModule::onSampleRateChange()
{
    float rate = engineGetSampleRate();
    tremolo.setSampleRate(rate);
}

TremoloModule::TremoloModule()
 : Module(tremolo.NUM_PARAMS,
    tremolo.NUM_INPUTS,
    tremolo.NUM_OUTPUTS,
    tremolo.NUM_LIGHTS),
    tremolo(this)
{
    onSampleRateChange();
    tremolo.init();
}

void TremoloModule::step()
{
    tremolo.step();
}

////////////////////
// module widget
////////////////////

struct TremoloWidget : ModuleWidget
{
    TremoloWidget(TremoloModule *);
       
    void addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
    }
};



/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
TremoloWidget::TremoloWidget(TremoloModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    const float rowIO = 330;
    addInput(Port::create<PJ301MPort>(Vec(10, rowIO), Port::INPUT, module, module->tremolo.AUDIO_INPUT));
    addLabel(Vec(8, rowIO-20), "in");
    
    addInput(Port::create<PJ301MPort>(Vec(10, rowIO-50), Port::INPUT, module, module->tremolo.CLOCK_INPUT));
    addLabel(Vec(5, rowIO-70), "ckin");
   
    addOutput(Port::create<PJ301MPort>(Vec(60, rowIO), Port::OUTPUT, module, module->tremolo.AUDIO_OUTPUT));
    addLabel(Vec(40, rowIO+10), "out");
    
    addOutput(Port::create<PJ301MPort>(Vec(60, rowIO-70), Port::OUTPUT, module, module->tremolo.SAW_OUTPUT));
    addLabel(Vec(50, rowIO-90), "saw");

    addOutput(Port::create<PJ301MPort>(Vec(60, rowIO-40), Port::OUTPUT, module, module->tremolo.LFO_OUTPUT));
    addLabel(Vec(35, rowIO-40), "lfo");

    // main params
    const float knobX = 10;
    const float knobY = 40;
    const float textX = 40;
    const float knobDy = 35;
// RoundLargeBlackKnob

    addParam(ParamWidget::create<RoundBlackKnob>(
        Vec(knobX, knobY), module, module->tremolo.LFO_RATE_PARAM, -5.0, 5.0, 0.0));
    addLabel(Vec(textX, knobY), "Rate");

    addParam(ParamWidget::create<RoundBlackKnob>(
        Vec(knobX, knobY + 1*knobDy), module, module->tremolo.LFO_SHAPE_PARAM, -5.0, 5.0, 0.0));
    addLabel(Vec(textX, knobY+1*knobDy), "Shape");

    addParam(ParamWidget::create<RoundBlackKnob>(
        Vec(knobX, knobY + 2*knobDy), module, module->tremolo.LFO_SKEW_PARAM, -5.0, 5.0, 0.0));
    addLabel(Vec(textX, knobY+2*knobDy), "Skew");
    
    addParam(ParamWidget::create<RoundBlackKnob>(
        Vec(knobX, knobY + 3*knobDy), module, module->tremolo.LFO_PHASE_PARAM, -5.0, 5.0, 0.0));
    addLabel(Vec(textX, knobY+3*knobDy), "Phase");


    addParam(ParamWidget::create<RoundBlackKnob>(
        Vec(knobX, knobY + 4*knobDy), module, module->tremolo.MOD_DEPTH_PARAM, -5.0, 5.0, 0.0));
    addLabel(Vec(textX, knobY+4*knobDy), "Depth");

    addParam(ParamWidget::create<RoundBlackSnapKnob>(
        Vec(knobX, knobY + 5*knobDy), module, module->tremolo.CLOCK_MULT_PARAM, 0.0f, 4.0f, 0.0f));
    addLabel(Vec(textX, knobY+5*knobDy), "Clock");

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
Model *modelTremoloModule = Model::create<TremoloModule,
     TremoloWidget>("Squinky Labs",
    "squinkylabs-tremolo",
    "Tremolo", EFFECT_TAG);

