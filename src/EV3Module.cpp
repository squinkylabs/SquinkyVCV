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
    Vec pos;
    pos.x = 10 + index * 40;
    pos.y = 50;

    const int delta = module->ev3.OCTAVE2_PARAM - module->ev3.OCTAVE1_PARAM;

    addParam(ParamWidget::create<RoundBlackSnapKnob>(
        Vec(pos.x, pos.y + 20), module, module->ev3.OCTAVE1_PARAM + delta * index,
        -5.0f, 4.0f, 0.f));
    addLabel(Vec(pos.x, pos.y), "Oct");

    addParam(ParamWidget::create<RoundBlackSnapKnob>(
        Vec(pos.x, pos.y + 70), module, module->ev3.SEMI1_PARAM + delta * index,
        0.f, 11.0f, 0.f));
    addLabel(Vec(pos.x, pos.y +50), "Semi");

    addParam(ParamWidget::create<Trimpot>(
        Vec(pos.x, pos.y+120), module, module->ev3.FINE1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    addLabel(Vec(pos.x, pos.y + 100), "Fine");

    // sync switches
    if (index != 0) {    
        addParam(ParamWidget::create<CKSS>(
            Vec(pos.x, pos.y + 160), module, module->ev3.SYNC1_PARAM + delta * index,
            0.0f, 1.0f, 1.0f));
        addLabel(Vec(pos.x-6, pos.y + 138), "on");
        addLabel(Vec(pos.x-6, pos.y + 190), "off");
    }

    addParam(ParamWidget::create<BlueToggle>(
        Vec(pos.x, pos.y + 220),
        module,
        EV3<WidgetComposite>::SAW1_PARAM + delta * index, 0.0f, 1.0f, 0.0f));
	

}

void EV3Widget::makeSections(EV3Module *module)
{
    makeSection(module, 0);
    makeSection(module, 1);
    makeSection(module, 2);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
EV3Widget::EV3Widget(EV3Module *module) : 
    ModuleWidget(module)
{
    box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    makeSections(module);

    addOutput(Port::create<PJ301MPort>(
        Vec(140, 330), Port::OUTPUT, module, module->ev3.MIX_OUTPUT));
    addLabel(Vec(130, 310), "Out");

    addInput(Port::create<PJ301MPort>(
        Vec(20, 330), Port::INPUT, module, module->ev3.CV1_INPUT));
    addLabel(Vec(20, 310), "CV");

    // TODO: suport create
    WaveformSelector* p = new WaveformSelector();
    p->box.pos = Vec(20, 280);
    addChild(p);
}



Model *modelEV3Module = Model::create<EV3Module,
    EV3Widget>("Squinky Labs",
    "squinkylabs-ev3",
    "EV3", EFFECT_TAG, OSCILLATOR_TAG);

#endif

