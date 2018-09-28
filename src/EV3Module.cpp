#include "Squinky.hpp"
#include "WaveformSelector.h"
#include "SQWidgets.h"
#include "WidgetComposite.h"

#ifdef _EV3

#include "EV3.h"



struct EV3Module : Module
{
    EV3Module();
    void step() override;
    EV3<WidgetComposite> ev3;
};

EV3Module::EV3Module()
    : Module(ev3.NUM_PARAMS,
    ev3.NUM_INPUTS,
    ev3.NUM_OUTPUTS,
    ev3.NUM_LIGHTS),
    ev3(this)
{
}

void EV3Module::step()
{
    ev3.step();
}

struct EV3Widget : ModuleWidget
{
    EV3Widget(EV3Module *);
    void makeSections(EV3Module *);
    void makeSection(EV3Module *, int index);
    void makeInputs(EV3Module *);
    void makeInput(EV3Module* module, int row, int col, EV3<WidgetComposite>::InputIds input, const char* name);
    void makeOututs(EV3Module *);
    void addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
    }
};

void EV3Widget::makeSection(EV3Module *module, int index)
{

    const float x = 30 + index * 86;
    const float x2 = x + 36;
    const float y = 50;
    const float y2 = y + 56;
    const float y3 = y2 + 40;

    const int delta = module->ev3.OCTAVE2_PARAM - module->ev3.OCTAVE1_PARAM;

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x, y), module, module->ev3.OCTAVE1_PARAM + delta * index,
        -5.0f, 4.0f, 0.f));
    addLabel(Vec(x-20, y-36), "Oct");

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x2, y), module, module->ev3.SEMI1_PARAM + delta * index,
        0.f, 11.0f, 0.f));
    addLabel(Vec(x2-20, y-36), "Semi");

    
    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x, y2), module, module->ev3.FINE1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    addLabel(Vec(x-20, y2-36), "Fine");

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x2, y2), module, module->ev3.FM1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    addLabel(Vec(x2-20, y2-36), "Mod");

  
 //   const float dx = 24;
    const float dy = 30;
    const float x0 = x-6;
    addParam(createParamCentered<Trimpot>(
        Vec(x0, y3), module, module->ev3.PW1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    if (index == 0)
        addLabel(Vec(x0+10, y3-12), "PW");

    addParam(createParamCentered<Trimpot>(
        Vec(x0, y3+dy), module, module->ev3.PWM1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    if (index == 0)
        addLabel(Vec(x0+10, y3+dy-12), "PWM");

          // sync switches
    if (index != 0) {    
        addParam(ParamWidget::create<CKSS>(
            Vec(x+30,y3), module, module->ev3.SYNC1_PARAM + delta * index,
            0.0f, 1.0f, 1.0f));
        addLabel(Vec(x+22, y3-20), "on");
        addLabel(Vec(x+22, y3+20), "off");
    }

    const float y4 = y3+ 50;
    const float xx = x - 8;
        // include one extra wf - none
    const float numWaves = (float) EV3<WidgetComposite>::Waves::END;
    const float defWave =  (float) EV3<WidgetComposite>::Waves::SAW;
    addParam(ParamWidget::create<WaveformSelector>(
        Vec(xx, y4),
        module,
        EV3<WidgetComposite>::WAVE1_PARAM + delta * index,
        0.0f, numWaves, defWave));
}

void EV3Widget::makeSections(EV3Module* module)
{
    makeSection(module, 0);
    makeSection(module, 1);
    makeSection(module, 2);
}

void EV3Widget::makeInput(EV3Module* module, int row, int col,  EV3<WidgetComposite>::InputIds input, const char* name)
{
    const float y = 280 + row * 30;
    const float x = 20 + col * 40;
    addInput(Port::create<PJ301MPort>(
        Vec(x, y), Port::INPUT, module, input));
    if (row == 0)
        addLabel(Vec(x, y-20), name);
}


void EV3Widget::makeInputs(EV3Module* module)
{
    for (int row=0; row<3; ++row) {
        makeInput(module, row, 0,  EV3<WidgetComposite>::CV1_INPUT, "V/oct");  
        makeInput(module, row, 1,  EV3<WidgetComposite>::CV1_INPUT, "Fm");  
        makeInput(module, row, 2,  EV3<WidgetComposite>::CV1_INPUT, "Pwm");  
    }
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
EV3Widget::EV3Widget(EV3Module *module) : 
    ModuleWidget(module)
{
    box.size = Vec(18 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    } 

    makeSections(module);
    makeInputs(module);

    addOutput(Port::create<PJ301MPort>(
        Vec(180, 280), Port::OUTPUT, module, module->ev3.MIX_OUTPUT));
    addLabel(Vec(170, 260), "Out");
#if 0
    addInput(Port::create<PJ301MPort>(
        Vec(20, 330), Port::INPUT, module, module->ev3.CV1_INPUT));
    addLabel(Vec(20, 310), "CV");
#endif
}



Model *modelEV3Module = Model::create<EV3Module,
    EV3Widget>("Squinky Labs",
    "squinkylabs-ev3",
    "EV3", EFFECT_TAG, OSCILLATOR_TAG);

#endif

