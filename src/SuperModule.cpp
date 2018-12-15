#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "SQWidgets.h"

#include "Super.h"

/**
 */
struct SuperModule : Module
{
public:
    SuperModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Super<WidgetComposite> super;
private:

};

void SuperModule::onSampleRateChange()
{
}

SuperModule::SuperModule()
    : Module(super.NUM_PARAMS,
    super.NUM_INPUTS,
    super.NUM_OUTPUTS,
    super.NUM_LIGHTS),
    super(this)
{
    onSampleRateChange();
    super.init();
}

void SuperModule::step()
{
    super.step();
}

////////////////////
// module widget
////////////////////

struct superWidget : ModuleWidget
{
    superWidget(SuperModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addPitchKnobs(SuperModule *);
    void addOtherKnobs(SuperModule *);
    void addJacks(SuperModule *);

};

const float width = 150;
const float col1 = 40;
const float middle = width / 2;
const float col2 = 110;

const float row1 = 71;
const float row2 = 134;
const float row3 = 220;
const float row4 = 250;

const float jackRow1 = 290;
const float jackRow2 = 332;

const float labelOffsetBig = -40;
const float labelOffsetSmall = -32;

void superWidget::addPitchKnobs(SuperModule *)
{
    // Octave
    auto oct = createParamCentered<Rogan1PSBlue>(
        Vec(col1, row1), module, Super<WidgetComposite>::OCTAVE_PARAM, -5, 4, 0);
    oct->snap = true;
    oct->smooth = false;
    addParam(oct);
    addLabel(
        Vec(col1 - 20, row1 + labelOffsetBig), "Oct");

    // Semi
    auto semi = createParamCentered<Rogan1PSBlue>(
        Vec(col2, row1), module, Super<WidgetComposite>::SEMI_PARAM, -11, 11, 0); 
    semi->snap = true;
    semi->smooth = false;
    addParam(semi);
    addLabel(
        Vec(col2 - 20, row1 + labelOffsetBig), "Semi");

    // Fine
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col1, row2), module, Super<WidgetComposite>::FINE_PARAM, -1, 1, 0));
    addLabel(
        Vec(col1 - 20, row2 + labelOffsetBig), "Fine");
    
    // FM
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col2, row2), module, Super<WidgetComposite>::FM_PARAM, 0, 1, 0));
    addLabel(Vec(col2 - 20, row2 + labelOffsetBig), "FM");

}

void superWidget::addOtherKnobs(SuperModule *)
{
    // Detune
    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, row3), module, Super<WidgetComposite>::DETUNE_PARAM, -5, 5, 0));
    addLabel(
        Vec(col1 - 26, row3 + labelOffsetSmall), "Detune");
    addParam(createParamCentered<Trimpot>(
        Vec(col1, row4), module, Super<WidgetComposite>::DETUNE_TRIM_PARAM, -1, 1, 0));

    addParam(createParamCentered<Blue30Knob>(
        Vec(col2, row3), module, Super<WidgetComposite>::MIX_PARAM, -5, 5, 0));
    addLabel(
        Vec(col2 - 20, row3 + labelOffsetSmall), "Mix");
    addParam(createParamCentered<Trimpot>(
        Vec(col2, row4), module, Super<WidgetComposite>::MIX_TRIM_PARAM, -1, 1, 0));

}

const float jackX = 27;
const float jackDx = 33;
const float jackOffsetLabel = -30;
const float jackLabelPoints = 11;


void superWidget::addJacks(SuperModule *)
{
    Label* l = nullptr;
    // first row
    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX, jackRow1),
        module,
        Super<WidgetComposite>::DETUNE_INPUT));
    l = addLabel(Vec(jackX - 24, jackRow1 + jackOffsetLabel), "Detune");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 3 * jackDx, jackRow1),
        module,
        Super<WidgetComposite>::MIX_INPUT));
    l = addLabel(Vec(jackX + 3 * jackDx - 17, jackRow1 + jackOffsetLabel), "Mix");
    l->fontSize = jackLabelPoints;

    // second row
    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX, jackRow2),
        module,
        Super<WidgetComposite>::CV_INPUT));
    l = addLabel(
        Vec(jackX - 16, jackRow2 + jackOffsetLabel), "V/8");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 1 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::TRIGGER_INPUT));
    l = addLabel(
        Vec(jackX + 1 * jackDx - 16,  jackRow2 + jackOffsetLabel), "Trig");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 2 * jackDx,  jackRow2),
        module,
        Super<WidgetComposite>::FM_INPUT));
    l = addLabel(
        Vec(jackX + 2 * jackDx - 13,  jackRow2 + jackOffsetLabel), "FM");
    l->fontSize = jackLabelPoints;

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jackX + 3 * jackDx,  jackRow2), 
         module,
        Super<WidgetComposite>::MAIN_OUTPUT));
    l = addLabel(
        Vec(jackX + 3 * jackDx - 16,  jackRow2 + jackOffsetLabel), "out");  
    l->fontSize = jackLabelPoints;
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
superWidget::superWidget(SuperModule *module) : ModuleWidget(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/super_panel.svg")));
        addChild(panel);
    }

    addPitchKnobs(module);
    addOtherKnobs(module);
    addJacks(module);

#if 0
    addParam(createParamCentered<Blue30Knob>(
        Vec(100, 60), module, Super<WidgetComposite>::FM_PARAM, 0, 1, 0));
    addLabel(Vec(col2+6, 25), "FM");
    addInput(createInputCentered<PJ301MPort>(
        Vec(col3-20, 100), module, Super<WidgetComposite>::FM_INPUT));
#endif

#if 0
    addParam(createParamCentered<NKK>(
        Vec(col2+ 25,170),
        module, 
        Super<WidgetComposite>::CLEAN_PARAM,
        0.0f, 2.0f, 0.0f
        ));
        addLabel(Vec(col2, 190), "classic");
        addLabel(Vec(col2+10, 130), "8X");
        #endif

#if 0
    // Semi
    const float semiY = 120;

    auto semi = createParamCentered<Rogan1PSBlue>(
        Vec(col1, semiY), module, Super<WidgetComposite>::SEMI_PARAM, -11, 11, 0); 
    semi->snap = true;
    semi->smooth = false;
    addParam(semi);
    addLabel(
        Vec(10, semiY + labelOffset), "Semi");

    // Fine
    const float fineY = 180;
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col1, fineY), module, Super<WidgetComposite>::FINE_PARAM, -1, 1, 0));
    addLabel(
        Vec(10, fineY + labelOffset), "Fine");

    // Detune
    const float detuneY = 240;
    labelOffset += 5;
    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, detuneY), module, Super<WidgetComposite>::DETUNE_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, detuneY + labelOffset), "Detune");
    addParam(createParamCentered<Trimpot>(
        Vec(col2, detuneY), module, Super<WidgetComposite>::DETUNE_TRIM_PARAM, -1, 1, 0));
    addInput(createInputCentered<PJ301MPort>(
        Vec(col3, detuneY), module, Super<WidgetComposite>::DETUNE_INPUT));

    // Waveform Mix
    const float mixY = 290;
    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, mixY), module, Super<WidgetComposite>::MIX_PARAM, -5, 5, 0));
    addLabel(
        Vec(10, mixY + labelOffset), "Mix");
    addParam(createParamCentered<Trimpot>(
        Vec(col2, mixY), module, Super<WidgetComposite>::MIX_TRIM_PARAM, -1, 1, 0));
    addInput(createInputCentered<PJ301MPort>(
        Vec(col3, mixY), module, Super<WidgetComposite>::MIX_INPUT));
#endif

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


Model *modelSuperModule = Model::create<SuperModule,
    superWidget>("Squinky Labs",
    "squinkylabs-super",
    "Saws: super saw VCO emulation", RANDOM_TAG);

