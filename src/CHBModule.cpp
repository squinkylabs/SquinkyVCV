#include "Squinky.hpp"
#include "SQWidgets.h"
#include "WidgetComposite.h"

#include <sstream>

#include "CHB.h"
#include "IMWidgets.hpp"

struct CHBModule;

class CHBPitchDisplay
{
public:
    CHBPitchDisplay(CHBModule * mod) : module(mod) {}
    void step();
   


    void addSemiLabel(Label*);

private:
    CHBModule * const module;


    Label* semiLabel=nullptr;
    float semiX=0;
 
    float lastSemi = 100;
    void update(int);
};

void CHBPitchDisplay::addSemiLabel(Label* l)
{
    semiLabel = l;
    semiX = l->box.pos.x;
}

static const char* names[] = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "D"
};

static int semi_offsets[] = {
    0,
 0,
 0,
 0,
 0,
  0,
  0,
 0,
 0,
 0,
 0,
 0
};



/**
 */
struct CHBModule : Module
{
public:
    CHBModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    CHB<WidgetComposite> chb;
private:
};

CHBModule::CHBModule()
    : Module(chb.NUM_PARAMS,
    chb.NUM_INPUTS,
    chb.NUM_OUTPUTS,
    chb.NUM_LIGHTS),
    chb(this)
{
}

void CHBModule::step()
{
    chb.step();
}

void CHBModule::onSampleRateChange()
{
    chb.onSampleRateChange();
}

void CHBPitchDisplay::step()
{

    const int semiParam = CHB<WidgetComposite>::PARAM_SEMIS;
    const int semi = module->params[semiParam].value;
    if (semi != lastSemi) {
        lastSemi = semi;
        std::stringstream so;
        int semi = lastSemi;
        if (semi < 0) {
            semi += 12;
        }

        so << "Semi: " << names[semi];
       // semiLabel->text = names[semi];
        semiLabel->text = so.str();
        semiLabel->box.pos.x = semiX + semi_offsets[semi];
    }

}

////////////////////
// module widget
////////////////////

struct CHBWidget : ModuleWidget
{
    friend struct CHBEconomyItem;
    CHBWidget(CHBModule *);

    /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

     void step() override
     {
         ModuleWidget::step();
         pitchDisplay.step();
     }
private:
    void addHarmonics(CHBModule *module);
    void addRow1(CHBModule *module);
    void addRow2(CHBModule *module);
    void addRow3(CHBModule *module);
    void addRow4(CHBModule *module);

    void addBottomJacks(CHBModule *module);
    void resetMe(CHBModule *module);

    // This is the gain which when run throught all the lookup tables
    // gives a gain of 1.
    const float defaultGainParam = .63108f;

    const int numHarmonics;
    CHBModule* const module;
    std::vector<ParamWidget* > harmonicParams;
    std::vector<float> harmonicParamMemory;
    ParamWidget* gainParam=nullptr;

     CHBPitchDisplay pitchDisplay;
};


/**
 * Global coordinate constants
 */
const float colHarmonicsJacks = 21;
const float rowFirstHarmonicJackY = 47;
const float harmonicJackSpacing = 32;
const float harmonicTrimDeltax = 27.5;

// columns of knobs
const float col1 = 95;
const float col2 = 150;
const float col3 = 214;
const float col4 = 268;

// rows of knobs
const float row1 = 75;
const float row2 = 131;
const float row3 = 201;
const float row4 = 237;
const float row5 = 287;
const float row6 = 332;

const float labelAboveKnob = 33;
const float labelAboveJack = 30;

inline void CHBWidget::addHarmonics(CHBModule *module)
{
    for (int i = 0; i < numHarmonics; ++i) {
        const float row = rowFirstHarmonicJackY + i * harmonicJackSpacing;
        addInput(createInputCentered<PJ301MPort>(
            Vec(colHarmonicsJacks, row),
            module,
            module->chb.H0_INPUT + i));

        const float defaultValue = (i == 0) ? 1 : 0;
        auto p = createParamCentered<Trimpot>(
            Vec(colHarmonicsJacks + harmonicTrimDeltax, row),
            module,
            module->chb.PARAM_H0 + i,
            0.0f, 1.0f, defaultValue);
        addParam(p);
        harmonicParams.push_back(p);
    }
}

void CHBWidget::addRow1(CHBModule *module)
{
    const float row = row1;

    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_RISE,
        -5.f, 5.f, 0.f));
    addLabel(Vec(col1 - 20, row - labelAboveKnob), "Rise");   

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_OCTAVE,
        -5.0f, 4.0f, 0.f));
    addLabel(Vec(col2 - 27, row1 - labelAboveKnob), "Octave");

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_SEMIS,
        -11.0f, 11.0f, 0.f));
    pitchDisplay.addSemiLabel( 
        addLabel(Vec(col3 - 30, row - labelAboveKnob), "Semi"));

    addParam(createParamCentered<Blue30Knob>(
        Vec(col4, row1),
        module,
        CHB<WidgetComposite>::PARAM_TUNE,
        -7.0f, 7.0f, 0));
    addLabel(Vec(col4 - 22, row1 - labelAboveKnob), "Tune");
}

void CHBWidget::addRow2(CHBModule *module)
{
    const float row = row2;

    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_FALL,
        -5.f, 5.f, 0.f));
    addLabel(Vec(col1 - 20, row - labelAboveKnob), "Fall");

    addParam(createParamCentered<Blue30Knob>(
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_PITCH_MOD_TRIM,
        0, 1.0f, 0.0f));
    addLabel(Vec(col3 - 20, row - labelAboveKnob), "Mod");

    addParam(createParamCentered<Blue30Knob>(
        Vec(col4, row),
        module,
        CHB<WidgetComposite>::PARAM_LINEAR_FM_TRIM,
        0, 1.0f, 0.0f));
    addLabel(Vec(col4 - 20, row - labelAboveKnob), "LFM");

    addParam(createParamCentered<CKSS>(
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_FOLD,
        0.0f, 1.0f, 0.0f));
    auto l = addLabel(Vec(col2 - 18, row - 30), "Fold");
    l->fontSize = 11;
    l = addLabel(Vec(col2 - 17, row + 10), "Clip");
    l->fontSize = 11;

    addChild(createLightCentered<SmallLight<GreenRedLight>>(
        Vec(col2-16, row),
        module,
        CHB<WidgetComposite>::GAIN_GREEN_LIGHT));
}

void CHBWidget::addRow3(CHBModule *module)
{
    const float row = row3;

    gainParam = createParamCentered<Blue30Knob>(
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_EXTGAIN,
        -5.0f, 5.0f, defaultGainParam);
    addParam(gainParam);
    addLabel(Vec(col1 - 22, row - labelAboveKnob), "Gain");

    //even
    addParam(createParamCentered<Blue30Knob>(
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_MAG_EVEN,
        -5, 5, 5));
    addLabel(Vec(col2 - 21.5, row - labelAboveKnob), "Even");

    // slope
    addParam(createParamCentered<Blue30Knob>(
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_SLOPE,
        -5, 5, 5));
    addLabel(Vec(col3 - 23, row - labelAboveKnob), "Slope");

    //odd
    addParam(createParamCentered<Blue30Knob>(
        Vec(col4, row),
        module,
        CHB<WidgetComposite>::PARAM_MAG_ODD,
        -5, 5, 5));
    addLabel(Vec(col4 - 20, row - labelAboveKnob), "Odd");

}

void CHBWidget::addRow4(CHBModule *module)
{
    float row = row4;

    addParam(createParamCentered<Trimpot>(
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_EXTGAIN_TRIM,
        0, 1, 0));

    addParam(createParamCentered<Trimpot>(
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_EVEN_TRIM,
        -1.0f, 1.0f, 0));

    addParam(createParamCentered<Trimpot>(
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_SLOPE_TRIM,
        -1.0f, 1.0f, 0));

    addParam(createParamCentered<Trimpot>(
        Vec(col4, row),
        module,
        CHB<WidgetComposite>::PARAM_ODD_TRIM,
        -1.0f, 1.0f, 0));
}

static const char* labels[] = {
    "V/Oct",
    "Mod",
    "LFM",
    "Even",
    "Slope",
    "Odd",
    "Ext In",
    "Gain",
    "EG",
    "Rise",
    "Fall",
    "Out",
    nullptr,
};
static const int offsets[] = {
    -1,
    1,
    2,
    0,
    -1,
    0,
    -1,
    1,
    5,
    0,
    0,
    2
};

static const int ids[] = {
    // top row
    CHB<WidgetComposite>::CV_INPUT,
    CHB<WidgetComposite>::PITCH_MOD_INPUT,
    CHB<WidgetComposite>::LINEAR_FM_INPUT,
    CHB<WidgetComposite>::EVEN_INPUT,
    CHB<WidgetComposite>::SLOPE_INPUT,
    CHB<WidgetComposite>::ODD_INPUT,
    //bottom row
    CHB<WidgetComposite>::AUDIO_INPUT,
    CHB<WidgetComposite>::GAIN_INPUT,
    CHB<WidgetComposite>::ENV_INPUT,
    CHB<WidgetComposite>::RISE_INPUT,
    CHB<WidgetComposite>::FALL_INPUT,
    CHB<WidgetComposite>::MIX_OUTPUT
};

void CHBWidget::addBottomJacks(CHBModule *module)
{
    const float jackCol1 = 93;
    const int numCol = 6;
    const float deltaX = 36;
    for (int jackRow = 0; jackRow < 2; ++jackRow) {
        for (int jackCol = 0; jackCol < numCol; ++jackCol) {
            const Vec pos(jackCol1 + deltaX * jackCol,
                jackRow == 0 ? row5 : row6);
            const int index = jackCol + numCol * jackRow;

            auto color = COLOR_BLACK;
            if (index == 11) {
                color = COLOR_WHITE;
            }

            const int id = ids[index];
            if (index == 11) {
                addOutput(createOutputCentered<PJ301MPort>(
                    pos,
                    module,
                    id));
            } else {
                addInput(createInputCentered<PJ301MPort>(
                    pos,
                    module,
                    id));
            }

            auto l = addLabel(Vec(pos.x - 20 + offsets[index], pos.y - labelAboveJack),
                labels[index],
                color);
            l->fontSize = 11;
        }
    }
}

void CHBWidget::resetMe(CHBModule *module)
{
    bool isOnlyFundamental = true;
    bool isAll = true;
    bool havePreset = !harmonicParamMemory.empty();
    const float val0 = harmonicParams[0]->value;
    if (val0 < .99) {
        isOnlyFundamental = false;
        isAll = false;
    }

    for (int i = 1; i < numHarmonics; ++i) {
        const float value = harmonicParams[i]->value;
        if (value < .9) {
            isAll = false;
        }

        if (value > .1) {
            isOnlyFundamental = false;
        }
    }

    if (!isOnlyFundamental && !isAll) {
        // take snapshot
        if (harmonicParamMemory.empty()) {
            harmonicParamMemory.resize(numHarmonics);
        }
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParamMemory[i] = harmonicParams[i]->value;
        }
    }

    // fundamental -> all
    if (isOnlyFundamental) {
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParams[i]->setValue(1);
        }
    }
    // all -> preset, if any
    else if (isAll && havePreset) {
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParams[i]->setValue(harmonicParamMemory[i]);
        }
    }
    // preset -> fund. if no preset all -> fund
    else {
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParams[i]->setValue((i == 0) ? 1 : 0);
        }
    }

    gainParam->setValue(defaultGainParam);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
CHBWidget::CHBWidget(CHBModule *module) :
    ModuleWidget(module),
    numHarmonics(module->chb.numHarmonics),
    module(module),
    pitchDisplay(module)
{
    box.size = Vec(20 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/chb_panel.svg")));
        addChild(panel);
 
        auto border = new PanelBorderWidget();
        border->box = box;
        addChild(border);
    }

    addHarmonics(module);
    addRow1(module);
    addRow2(module);
    addRow3(module);
    addRow4(module);

    auto sw = new SQPush(
        "res/preset-button-up.svg",
        "res/preset-button-down.svg");
    Vec pos(64, 360);
    sw->center(pos);
    sw->onClick([this, module]() {
        this->resetMe(module);
    });

    addChild(sw);
    addBottomJacks(module);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 
}

Model *modelCHBModule = Model::create<CHBModule,
    CHBWidget>("Squinky Labs",
    "squinkylabs-CHB2",
    "Chebyshev II: Waveshaper VCO", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);
