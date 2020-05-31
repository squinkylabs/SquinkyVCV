
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SUB
#include "Sub.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/PopupMenuParamWidgetv1.h"

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
    DECLARE_MANUAL("Blank Manul", "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/booty-shifter.md");

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
    void addJacks(SubModule *module, std::shared_ptr<IComposite> icomp);
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

const float labelAboveKnob = 20;

const float knobX1Trim = knobX1 + 14;
const float knobX3Trim = knobX1Trim + + 2 * knobDeltaX;;

const float knob2XOffset = 144;

/** 
 * side = 0 for left / 1
 *      1 for right / 2
 */
void SubWidget::addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp, int side)
{
    assert(side >= 0 && side <= 1);
    const float xOffset = side ? knob2XOffset : 0;

    // first row
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX1+xOffset, knobY1),
        module,
        Comp::OCTAVE1_PARAM + side));
    addLabel(Vec(knobX1+xOffset - 13, knobY1 - labelAboveKnob), "Octave");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2+xOffset, knobY1),
        module,
        Comp::FINE1_PARAM + side));
    addLabel(Vec(knobX2+xOffset - 4, knobY1 - labelAboveKnob), 
        side ? "Fine 2" : "Fine 1");

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX3Trim+xOffset - 18, knobY1 + 4),
        module,
        Comp::WAVEFORM1_PARAM + side);
    p->box.size.x = 44;  // width
    p->box.size.y = 22;      // should set auto like button does
    p->text = "Saw";
    p->setLabels( {"Saw", "Sq", "Mix"});
    addParam(p);

    // second row

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX1+xOffset, knobY2),
        module,
        Comp::VCO1_LEVEL_PARAM + side));
    addLabel(Vec(knobX1+xOffset - 8, knobY2 - labelAboveKnob), 
        side ? "Vol 2" : "Vol 1");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2+xOffset, knobY2),
        module,
        Comp::SUB1A_LEVEL_PARAM + side));
    addLabel(Vec(knobX2+xOffset - 8, knobY2 - labelAboveKnob), 
        side ? "Sub 2A" : "Sub A1");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3+xOffset, knobY2),
        module,
        Comp::SUB1B_LEVEL_PARAM + side));
    addLabel(Vec(knobX3+xOffset - 8, knobY2 - labelAboveKnob), 
        side ? "Sub 2B" : "Sub 1B");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2+xOffset, knobY3),
        module,
        Comp::SUB1A_TUNE_PARAM + side));
    addLabel(Vec(knobX2+xOffset - 4, knobY3 - labelAboveKnob), 
        side ? "Div 2A" : "Div 1A");

     addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3+xOffset, knobY3),
        module,
        Comp::SUB1B_TUNE_PARAM + side));
    addLabel(Vec(knobX3+xOffset - 4, knobY3 - labelAboveKnob), 
        side ? "Div 2B" : "Div 1B");

}

const float jacksX1 = 24;
const float jacksDeltaX = 38;
const float jacksX2 = jacksX1 + 1 * jacksDeltaX;
const float jacksX3 = jacksX1 + 2 * jacksDeltaX;
const float jacksX4 = jacksX1 + 3 * jacksDeltaX;
const float jacksX5 = jacksX1 + 4 * jacksDeltaX;

const float jacksY1 = 274; // 230;
const float jacksY2 = 322; // 330; //300;

const float jacksX1Top = 28;
const float jacksX3Top = jacksX1Top + 2 * knobDeltaX;


void SubWidget::addJacks(SubModule *module, std::shared_ptr<IComposite> icomp)
{
    addInput(createInput<PJ301MPort>(
        Vec(jacksX1Top, jacksY1),
        module,
        Comp::SUB1A_TUNE_INPUT));
    addInput(createInput<PJ301MPort>(
        Vec(jacksX3Top, jacksY1),
        module,
        Comp::SUB2A_TUNE_INPUT));
 //   addLabel(Vec(jacksX1Top - 10, jacksY1 - labelAboveKnob), "Div 1");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY2),
        module,
        Comp::VOCT_INPUT));
    addLabel(Vec(jacksX1 - 10, jacksY2 - labelAboveKnob), "V/8");

    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX2, jacksY2),
        module,
        Comp::MAIN_OUTPUT));
    addLabel(Vec(jacksX2 - 10, jacksY2 - labelAboveKnob), "Out");
}




/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SubWidget::SubWidget(SubModule *module)
{
    setModule(module);

    box.size = Vec(20 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/wvco_panel.svg");

    addLabel(Vec(100, 14), "Substitute");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addKnobs(module, icomp, 0);
    addKnobs(module, icomp, 1);
    addJacks(module, icomp);
}

Model *modelSubModule = createModel<SubModule, SubWidget>("squinkylabs-sub");
#endif

