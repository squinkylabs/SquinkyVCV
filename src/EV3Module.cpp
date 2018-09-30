#include "Squinky.hpp"
#include "WaveformSelector.h"
#include "SQWidgets.h"
#include "WidgetComposite.h"

#ifdef _EV3

#include "EV3.h"
#include <sstream>

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

/************************************************************/

class PitchDisplay
{
public:
    PitchDisplay(EV3Module * mod) : module(mod) {}
    void step();

    /**
     * Labels must be added in order
     */
    void addLabel(Label*);
private:
    EV3Module * const module;
    std::vector<Label*> labels;
    int lastOctave[3] = {100, 100, 100};
    int lastSemi[3] = {100, 100, 100};

    void update(int);
};

void PitchDisplay::step()
{
    const int delta = EV3<WidgetComposite>::OCTAVE2_PARAM - EV3<WidgetComposite>::OCTAVE1_PARAM;

    if (labels.size() != 3) {
        printf("display not init\n"); fflush(stdout);
    }

    for (int i=0; i<3; ++i) {
        const int octaveParam = EV3<WidgetComposite>::OCTAVE1_PARAM + delta * i;
        const int semiParam = EV3<WidgetComposite>::SEMI1_PARAM + delta * i;
        const int oct = module->params[octaveParam].value;
        const int semi = module->params[semiParam].value;
        if (semi != lastSemi[i] || oct != lastOctave[i]) {
            lastSemi[i] = semi;
            lastOctave[i] = oct;
            update(i);
        }
    }
}

static const char* names[] = {
    "-",
    "m2nd",
    "2nd",
    "m3rd",
    "M3rd",
    "4th",
    "Dim5th",
    "5th",
    "m6th",
    "M6th",
    "m7th",
    "M7th",
    "oct"
};

void PitchDisplay::update(int osc) {
    std::stringstream s;
    int oct = 5 + lastOctave[osc];
    int semi = lastSemi[osc];
    switch(osc) {
        case 0:
            s << oct << ":" << semi;
            break;
        case 1:
        if (semi < 0) {
                --oct;
                semi += 12;
            }
            s << oct << ":" << semi;
            break;
        case 2:
            if (semi < 0) {
                --oct;
                semi += 12;
            }
            s << oct << ":" << names[semi];
            break;
    }
    labels[osc]->text = s.str();
}

void PitchDisplay::addLabel(Label* l)
{
    labels.push_back(l);
}

struct EV3Widget : ModuleWidget
{
    EV3Widget(EV3Module *);
    void makeSections(EV3Module *);
    void makeSection(EV3Module *, int index);
    void makeInputs(EV3Module *);
    void makeInput(EV3Module* module, int row, int col, int input, const char* name);
    void makeOutputs(EV3Module *);
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void step() override;

    PitchDisplay pitchDisplay;
};

void EV3Widget::step()
{
    ModuleWidget::step();
    pitchDisplay.step();
}

void EV3Widget::makeSection(EV3Module *module, int index)
{
    const float x = 30 + index * 86;
    const float x2 = x + 36;
    const float y = 80;
    const float y2 = y + 56;
    const float y3 = y2 + 40;

    const int delta = EV3<WidgetComposite>::OCTAVE2_PARAM - EV3<WidgetComposite>::OCTAVE1_PARAM;

    pitchDisplay.addLabel( addLabel(
        Vec(x, y-58), "foo"
    ));
    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x, y), module,
        EV3<WidgetComposite>::OCTAVE1_PARAM + delta * index,
        -5.0f, 4.0f, 0.f));
    addLabel(Vec(x - 20, y - 36), "Oct");

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x2, y), module,
        EV3<WidgetComposite>::SEMI1_PARAM + delta * index,
        -11.f, 11.0f, 0.f));
    addLabel(Vec(x2 - 20, y - 36), "Semi");

    addParam(createParamCentered<Blue30Knob>(
        Vec(x, y2), module,
        EV3<WidgetComposite>::FINE1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    addLabel(Vec(x - 20, y2 - 36), "Fine");

    addParam(createParamCentered<Blue30Knob>(
        Vec(x2, y2), module,
        EV3<WidgetComposite>::FM1_PARAM + delta * index,
        0.f, 1.f, 0));
    addLabel(Vec(x2 - 20, y2 - 36), "Mod");

    const float dy = 30;
    const float x0 = x - 6;
    addParam(createParamCentered<Trimpot>(
        Vec(x0, y3), module, EV3<WidgetComposite>::PW1_PARAM + delta * index,
        -1.f, 1.f, 0));
    if (index == 0)
        addLabel(Vec(x0 + 10, y3 - 12), "PW");

    addParam(createParamCentered<Trimpot>(
        Vec(x0, y3 + dy), module,
        EV3<WidgetComposite>::PWM1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    if (index == 0)
        addLabel(Vec(x0 + 10, y3 + dy - 12), "PWM");

          // sync switches
    if (index != 0) {
        addParam(ParamWidget::create<CKSS>(
            Vec(x + 30, y3), module, EV3<WidgetComposite>::SYNC1_PARAM + delta * index,
            0.0f, 1.0f, 0.0f));
        addLabel(Vec(x + 22, y3 - 20), "on");
        addLabel(Vec(x + 22, y3 + 20), "off");
    }

    const float y4 = y3 + 50;
    const float xx = x - 8;
        // include one extra wf - none
    const float numWaves = (float) EV3<WidgetComposite>::Waves::END;
    const float defWave = (float) EV3<WidgetComposite>::Waves::SIN;
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

const float row1Y = 280;
const float rowDY = 30;
const float colDX = 40;

void EV3Widget::makeInput(EV3Module* module, int row, int col,
    int inputNum, const char* name)
{
    EV3<WidgetComposite>::InputIds input = EV3<WidgetComposite>::InputIds(inputNum);
    const float y = row1Y + row * rowDY;
    const float x = 20 + col * colDX;
    const float labelX = x - 6;
    addInput(Port::create<PJ301MPort>(
        Vec(x, y), Port::INPUT, module, input));
    if (row == 0)
        addLabel(Vec(labelX, y - 20), name);
}

void EV3Widget::makeInputs(EV3Module* module)
{
    for (int row = 0; row < 3; ++row) {
        makeInput(module, row, 0, EV3<WidgetComposite>::CV1_INPUT + row, "V/oct");
        makeInput(module, row, 1, EV3<WidgetComposite>::FM1_INPUT + row, "Fm");
        makeInput(module, row, 2, EV3<WidgetComposite>::PWM1_INPUT + row, "Pwm");
    }
}

void EV3Widget::makeOutputs(EV3Module *)
{
    const float x = 160;
    const float trimY = row1Y + 11;
    const float outX = x + 30;

    addLabel(Vec(x, trimY - 30), ".... outputs ....");

    addParam(createParamCentered<Trimpot>(
        Vec(x, trimY), module, EV3<WidgetComposite>::MIX1_PARAM,
        0.0f, 1.0f, 0));
    addParam(createParamCentered<Trimpot>(
        Vec(x, trimY + rowDY), module, EV3<WidgetComposite>::MIX2_PARAM,
        0.0f, 1.0f, 0));
    addParam(createParamCentered<Trimpot>(
        Vec(x, trimY + 2 * rowDY), module, EV3<WidgetComposite>::MIX3_PARAM,
        0.0f, 1.0f, 0));

    addOutput(Port::create<PJ301MPort>(
        Vec(outX, row1Y),
        Port::OUTPUT, module, EV3<WidgetComposite>::VCO1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(outX, row1Y + rowDY),
        Port::OUTPUT, module, EV3<WidgetComposite>::VCO2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(
        Vec(outX, row1Y + 2 * rowDY),
        Port::OUTPUT, module, EV3<WidgetComposite>::VCO3_OUTPUT));

    addOutput(Port::create<PJ301MPort>(
        Vec(outX + colDX, row1Y + rowDY),
        Port::OUTPUT, module, EV3<WidgetComposite>::MIX_OUTPUT));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
EV3Widget::EV3Widget(EV3Module *module) :
    ModuleWidget(module),
    pitchDisplay(module)
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
    makeOutputs(module);
#if 0
    addOutput(Port::create<PJ301MPort>(
        Vec(180, 280), Port::OUTPUT, module, module->ev3.MIX_OUTPUT));
    addLabel(Vec(170, 260), "Out");

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

