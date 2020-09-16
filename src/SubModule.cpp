
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SUB
#include "Sub.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/SemitoneDisplay.h"
#include "ctrl/ToggleButton.h"

using Comp = Sub<WidgetComposite>;

/**
 */
struct SubModule : Module
{
public:
    SubModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void SubModule::onSampleRateChange()
{
}

SubModule::SubModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();
}

void SubModule::step()
{
    blank->step();
}

////////////////////
// module widget
////////////////////

struct SubWidget : ModuleWidget
{
    SemitoneDisplay semitoneDisplay1;
    SemitoneDisplay semitoneDisplay2;

    SubWidget(SubModule *);
    void appendContextMenu(Menu *menu) override;
    void step() override
    {
        ModuleWidget::step();
        semitoneDisplay1.step();
        semitoneDisplay2.step();
    }

    Label* addLabelx(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        label->fontSize = 14;
        addChild(label);
        return label;
    }

    void addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp, int side);
    void addJacks(SubModule *module, std::shared_ptr<IComposite> icomp, int side);
    void addMiddleControls(SubModule *module, std::shared_ptr<IComposite> icomp);
    void addMiddleJacks(SubModule *module, std::shared_ptr<IComposite> icomp);
};

void SubWidget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

    ManualMenuItem* manual = new ManualMenuItem(
        "Substitute manual",
        "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/substitute.md");
    menu->addChild(manual);
    
    SqMenuItem_BooleanParam2 * item = new SqMenuItem_BooleanParam2(module, Comp::AGC_PARAM);
    item->text = "AGC";
    menu->addChild(item);
}

const float knobLeftEdge = 18;
const float knobDeltaX = 46;
const float knobX1 = 14;
const float knobX2 =62;
const float knobX3 = 110;

const float knobX4 = 218;
const float knobX5 = 266;
const float knobX6 = 314;

const float knobY1 = 55;
const float knobDeltaY = 70;
const float knobY2 = 115;
const float knobY3 = 178;
const float knobY4 = 229;

const float labelAboveKnob = 20;

const float knobX1Trim = knobX1 + 14;
const float knobX3Trim = knobX1Trim + + 2 * knobDeltaX;;

const float knob2XOffset = 144;
const float trimXOffset = 5;

const float widthHP = 24;
const float totalWidth = widthHP * RACK_GRID_WIDTH;
const float middle = totalWidth / 2;

const float jacksX1 = 17;
const float jacksX2 = 65;
const float jacksX3 = 113;
const float jacksX4 = 221;
const float jacksX5 = 269;
const float jacksX6 = 317;

const float jacksY1 = 268; 
const float jacksY2 = 328; 

const float jackOffsetX = 3;
/** 
 * side = 0 for left / 1
 *      1 for right / 2
 */
void SubWidget::addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp, int side)
{
    assert(side >= 0 && side <= 1);
   // const float xOffset = side ? knob2XOffset : 0;
#if 0
    auto xfunc = [](float xOrig, int side) {
        if (side == 0) {
            return xOrig;
        } else {
            return totalWidth - (xOrig + 30);
        }
    };
#endif

    SemitoneDisplay& semiDisp = side ? semitoneDisplay2 : semitoneDisplay1;
    // first row
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(side ? knobX6 : knobX1, knobY1),
        module,
        Comp::OCTAVE1_PARAM + side));
    //addLabel(Vec(xfunc(knobX1, side) - 13, knobY1 - labelAboveKnob), "Octave");
    semiDisp.setOctLabel(
        addLabelx(Vec(side ? 311 : 11, 35), "Octave"),
        Comp::OCTAVE1_PARAM + side);

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(side ? knobX5 : knobX2, knobY1),
        module,
        Comp::SEMITONE1_PARAM + side));
    //addLabel(Vec(xfunc(knobX2, side) - 3, knobY1 - labelAboveKnob),  "Semi");
    semiDisp.setSemiLabel(
        addLabelx(Vec(side ? 262 : 49, knobY1 - labelAboveKnob), "Semi"),
        Comp::SEMITONE1_PARAM + side);

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(side ? knobX4 : knobX3, knobY1),
        module,
        Comp::FINE1_PARAM + side));
    //addLabel(Vec(xfunc(knobX3, side) - 3, knobY1 - labelAboveKnob),  "Fine");

    // second row

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(side ? knobX6 : knobX1, knobY2),
        module,
        Comp::VCO1_LEVEL_PARAM + side));
    //addLabel(Vec(xfunc(knobX1, side) - 4, knobY2 - labelAboveKnob),    "Vol");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(side ? knobX5 : knobX2, knobY2),
        module,
        Comp::SUB1A_LEVEL_PARAM + side));
    //addLabel(Vec(xfunc(knobX2, side) - 8, knobY2 - labelAboveKnob),   "Sub A");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(side ? knobX4 : knobX3, knobY2),
        module,
        Comp::SUB1B_LEVEL_PARAM + side));
    //addLabel(Vec(xfunc(knobX3, side) - 8, knobY2 - labelAboveKnob),   "Sub B");


    //----------------------- third row --------------------------
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(side ? knobX6 : knobX1, knobY3),
        module,
        Comp::PULSEWIDTH1_PARAM + side));
    //addLabel(Vec(xfunc(knobX1, side) - 6, knobY3 - labelAboveKnob),       "PW"); 
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(side ? knobX5 : knobX2, knobY3),
        module,
        Comp::SUB1A_TUNE_PARAM + side));
    //addLabel(Vec(xfunc(knobX2, side) - 6, knobY3 - labelAboveKnob),   "Div A");

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(side ? knobX4 : knobX3, knobY3),
        module,
        Comp::SUB1B_TUNE_PARAM + side));
    //addLabel(Vec(xfunc(knobX3, side) - 6, knobY3 - labelAboveKnob),   "Div B");


    // trimmers
    addParam(SqHelper::createParam<SqTrimpot24>(
        icomp,
        Vec(side ? jacksX6 : jacksX1, knobY4),
        module,
        Comp::PULSEWIDTH1_TRIM_PARAM + side));
    addParam(SqHelper::createParam<SqTrimpot24>(
        icomp,
        Vec(side ? jacksX5 : jacksX2, knobY4),
        module,
        Comp::SUB1A_TUNE_TRIM_PARAM + side));
    addParam(SqHelper::createParam<SqTrimpot24>(
        icomp,
        Vec(side ? jacksX4 : jacksX3, knobY4),
        module,
        Comp::SUB1B_TUNE_TRIM_PARAM + side));
}


const float xMiddleSel = middle -24;
void SubWidget::addMiddleControls(SubModule *module, std::shared_ptr<IComposite> icomp)
{
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(xMiddleSel, 206),
        module,
        Comp::QUANTIZER_SCALE_PARAM);
    p->box.size.x = 48;  // width
    p->box.size.y = 22;   
    p->text = "Off";
    p->setLabels( {"Off", "12ET", "7ET", "12JI", "7JI"});
    addParam(p);

    int side = 0;
    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(xMiddleSel, knobY1 - 10),
        module,
        Comp::WAVEFORM1_PARAM + side);
    p->box.size.x = 44;  // width
    p->box.size.y = 22;      // should set auto like button does
    p->text = "Saw";
    p->setLabels( {"Saw", "Sq", "Mix"});
    addParam(p);

    side = 1;
    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(xMiddleSel, knobY1 + 20),
        module,
        Comp::WAVEFORM1_PARAM + side);
    p->box.size.x = 44;  // width
    p->box.size.y = 22;      // should set auto like button does
    p->text = "Saw";
    p->setLabels( {"Saw", "Sq", "Mix"});
    addParam(p);
}


void SubWidget::addJacks(SubModule *module, std::shared_ptr<IComposite> icomp, int side)
{
    #if 0
    auto xfunc = [](float xOrig, int side) {
        if (side == 0) {
            return xOrig;
        } else {

            return totalWidth - (xOrig + 28);
        }
    };
    #endif

    //-------------- first row -------------------
    addInput(createInput<PJ301MPort>(
        Vec(side ? jacksX6 : jacksX1, jacksY1),
        module,
        Comp::PWM1_INPUT+side));

    addInput(createInput<PJ301MPort>(
        Vec(side ? jacksX5 : jacksX2, jacksY1),
        module,
        Comp::SUB1A_TUNE_INPUT+side));
    addInput(createInput<PJ301MPort>(
        Vec(side ? jacksX4 : jacksX3, jacksY1),
        module,
        Comp::SUB1B_TUNE_INPUT+side));

    //------------------ second row ------------------
    addInput(createInput<PJ301MPort>(
        Vec(side ? jacksX6 : jacksX1, jacksY2),
        module,
        Comp::MAIN1_LEVEL_INPUT+side));
    //addLabel(Vec(xfunc(knobX1, side) - 6, jacksY2 - labelAboveKnob),       "Vol");

    addInput(createInput<PJ301MPort>(
        Vec(side ? jacksX5 : jacksX2, jacksY2),
        module,
        Comp::SUB1A_LEVEL_INPUT+side));
    //addLabel(Vec(xfunc(knobX2, side) - 6, jacksY2 - labelAboveKnob),     "Sub A");

    addInput(createInput<PJ301MPort>(
        Vec(side ? jacksX4 : jacksX3, jacksY2),
        module,
        Comp::SUB1B_LEVEL_INPUT+side));
    //addLabel(Vec(xfunc(knobX3, side) - 6, jacksY2 - labelAboveKnob),      "Sub B");

}

void SubWidget::addMiddleJacks(SubModule *module, std::shared_ptr<IComposite> icomp) {
    const float jacksMiddle = 168;
    // middle ones
    addInput(createInput<PJ301MPort>(
        Vec(jacksMiddle, jacksY1),
        module,
        Comp::VOCT_INPUT));
    //addLabel(Vec(jacksMiddle - 10, jacksY1 - labelAboveKnob), "V/Oct");

    addOutput(createOutput<PJ301MPort>(
        Vec(jacksMiddle, jacksY2),
        module,
        Comp::MAIN_OUTPUT));
    //addLabel(Vec(jacksMiddle - 7, jacksY2 - labelAboveKnob), "Out");
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
SubWidget::SubWidget(SubModule *module) :   
    semitoneDisplay1(module), semitoneDisplay2(module)
{
    semitoneDisplay1.octaveDisplayOffset = -5;
    semitoneDisplay2.octaveDisplayOffset = -5;

    setModule(module);

    // box.size = Vec(totalWidth, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/substitute-panel.svg");

    //addLabel(Vec(140, 14), "Substitute");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addKnobs(module, icomp, 0);
    addKnobs(module, icomp, 1);
    addJacks(module, icomp, 0);
    addJacks(module, icomp, 1);
    addMiddleControls(module, icomp);
    addMiddleJacks(module, icomp);
}

Model *modelSubModule = createModel<SubModule, SubWidget>("squinkylabs-sub");
#endif

