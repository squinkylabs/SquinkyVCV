
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SUB
#include "Sub.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
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
    SubWidget(SubModule *);
    DECLARE_MANUAL("Substitute manual", "https://github.com/squinkylabs/SquinkyVCV/blob/ck-1/docs/substitute.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp, int side);
    void addJacks(SubModule *module, std::shared_ptr<IComposite> icomp, int side);
    void addMiddleControls(SubModule *module, std::shared_ptr<IComposite> icomp);
    void addMiddleJacks(SubModule *module, std::shared_ptr<IComposite> icomp);
};

const float knobLeftEdge = 18;
const float knobDeltaX = 46;
const float knobX1 = knobLeftEdge;
const float knobX2 = knobLeftEdge + 1 * knobDeltaX;
const float knobX3 = knobLeftEdge + 2 * knobDeltaX;
const float knobX4 = knobLeftEdge + 3 * knobDeltaX;

const float knobY1 = 60;
const float knobDeltaY = 70;
const float knobY2 = knobY1 + 1 *  knobDeltaY;
const float knobY3 = knobY1 + 2 *  knobDeltaY - 12;
const float knobY4 = knobY3 + 50;

const float labelAboveKnob = 20;

const float knobX1Trim = knobX1 + 14;
const float knobX3Trim = knobX1Trim + + 2 * knobDeltaX;;

const float knob2XOffset = 144;
const float trimXOffset = 5;

//const float centerWidth = 30;

const float widthHP = 24;
const float totalWidth = widthHP * RACK_GRID_WIDTH;
const float middle = totalWidth / 2;

/** 
 * side = 0 for left / 1
 *      1 for right / 2
 */
void SubWidget::addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp, int side)
{
    assert(side >= 0 && side <= 1);
   // const float xOffset = side ? knob2XOffset : 0;

    auto xfunc = [](float xOrig, int side) {
        if (side == 0) {
            return xOrig;
        } else {
            return totalWidth - (xOrig + 30);
        }
    };

    // first row
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(xfunc(knobX1, side), knobY1),
        module,
        Comp::OCTAVE1_PARAM + side));
    addLabel(Vec(xfunc(knobX1, side) - 13, knobY1 - labelAboveKnob), "Octave");

     addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(xfunc(knobX2, side), knobY1),
        module,
        Comp::SEMITONE1_PARAM + side));
    addLabel(Vec(xfunc(knobX2, side) - 3, knobY1 - labelAboveKnob),  "Semi");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(xfunc(knobX3, side), knobY1),
        module,
        Comp::FINE1_PARAM + side));
    addLabel(Vec(xfunc(knobX3, side) - 3, knobY1 - labelAboveKnob),  "Fine");

    // second row

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(xfunc(knobX1, side), knobY2),
        module,
        Comp::VCO1_LEVEL_PARAM + side));
    addLabel(Vec(xfunc(knobX1, side) - 4, knobY2 - labelAboveKnob), 
        "Vol");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(xfunc(knobX2, side), knobY2),
        module,
        Comp::SUB1A_LEVEL_PARAM + side));
    addLabel(Vec(xfunc(knobX2, side) - 8, knobY2 - labelAboveKnob), 
        "Sub A");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(xfunc(knobX3, side), knobY2),
        module,
        Comp::SUB1B_LEVEL_PARAM + side));
    addLabel(Vec(xfunc(knobX3, side) - 8, knobY2 - labelAboveKnob), 
        "Sub B");


//Blue30SnapKnob
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(xfunc(knobX2, side), knobY3),
        module,
        Comp::SUB1A_TUNE_PARAM + side));
    addLabel(Vec(xfunc(knobX2, side) - 6, knobY3 - labelAboveKnob), 
        "Div A");

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(xfunc(knobX3, side), knobY3),
        module,
        Comp::SUB1B_TUNE_PARAM + side));
    addLabel(Vec(xfunc(knobX3, side) - 6, knobY3 - labelAboveKnob), 
        "Div B");


    // trimmers
    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(xfunc(knobX2+1, side) +trimXOffset, knobY4),
        module,
        Comp::SUB1A_TUNE_TRIM_PARAM+ side));
    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(xfunc(knobX3+1, side) +trimXOffset, knobY4),
        module,
        Comp::SUB1B_TUNE_TRIM_PARAM+ side));
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

const float jacksX1 = 24;
const float jacksDeltaX = 38;
const float jacksX2 = jacksX1 + 1 * jacksDeltaX;
const float jacksX3 = jacksX1 + 2 * jacksDeltaX;
const float jacksX4 = jacksX1 + 3 * jacksDeltaX;
const float jacksX5 = jacksX1 + 4 * jacksDeltaX;

const float jacksY1 = 274; // 230;
const float jacksY2 = 322; // 330; //300;

//const float jacksX1Top = 28;
//const float jacksX3Top = jacksX1Top + 2 * knobDeltaX;


const float jackOffsetX = 3;
void SubWidget::addJacks(SubModule *module, std::shared_ptr<IComposite> icomp, int side)
{
    auto xfunc = [](float xOrig, int side) {
        if (side == 0) {
            return xOrig;
        } else {

            return totalWidth - (xOrig + 28);
        }
    };

    //-------------- first row -------------------
    addInput(createInput<PJ301MPort>(
        Vec(xfunc(knobX2, side), jacksY1),
        module,
        Comp::SUB1A_TUNE_INPUT+side));
    addInput(createInput<PJ301MPort>(
        Vec(xfunc(knobX3, side), jacksY1),
        module,
        Comp::SUB1B_TUNE_INPUT+side));
#if 0
    addInput(createInput<PJ301MPort>(
        Vec(xfunc(knobX2, 1), jacksY1),
        module,
        Comp::SUB2A_TUNE_INPUT));
    addInput(createInput<PJ301MPort>(
        Vec(xfunc(knobX3, 1), jacksY1),
        module,
        Comp::SUB2B_TUNE_INPUT));
#endif

    //------------------ second row ------------------
}
#if 0
    addInput(createInput<PJ301MPort>(
        Vec(knobX1+jackOffsetX, jacksY2),
        module,
        Comp::MAIN1_LEVEL_INPUT));
    addLabel(Vec(xfunc(knobX3, side) - 6, knobY3 - labelAboveKnob), 
        "Div B");
#endif



void SubWidget::addMiddleJacks(SubModule *module, std::shared_ptr<IComposite> icomp) {
    const float jacksMiddle = middle - 10;
    // middle ones
    addInput(createInput<PJ301MPort>(
        Vec(jacksMiddle, jacksY1),
        module,
        Comp::VOCT_INPUT));
    addLabel(Vec(jacksMiddle - 10, jacksY1 - labelAboveKnob), "V/Oct");

    addOutput(createOutput<PJ301MPort>(
        Vec(jacksMiddle, jacksY2),
        module,
        Comp::MAIN_OUTPUT));
    addLabel(Vec(jacksMiddle - 7, jacksY2 - labelAboveKnob), "Out");
}






/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SubWidget::SubWidget(SubModule *module)
{
    setModule(module);

    // box.size = Vec(totalWidth, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/sub_panel.svg");

    addLabel(Vec(150, 14), "Substitute");

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
}

Model *modelSubModule = createModel<SubModule, SubWidget>("squinkylabs-sub");
#endif

