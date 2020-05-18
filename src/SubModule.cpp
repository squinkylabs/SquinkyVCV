
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SUB
#include "Sub.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

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

    void addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp);
    void addJacks(SubModule *module, std::shared_ptr<IComposite> icomp);
};

const float knobLeftEdge = 24;
const float knobDeltaX = 46;
const float knobX1 = knobLeftEdge;
const float knobX2 = knobLeftEdge + 1 * knobDeltaX;
const float knobX3 = knobLeftEdge + 2 * knobDeltaX;
const float knobX4 = knobLeftEdge + 3 * knobDeltaX;

const float knobY1 = 60;
const float knobDeltaY = 70;
const float knobY2 = knobY1 + 1 *  knobDeltaY;
const float knobY3 = knobY1 + 2 *  knobDeltaY;

const float labelAboveKnob = 20;

void SubWidget::addKnobs(SubModule *module, std::shared_ptr<IComposite> icomp)
{

    // first row
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX1, knobY1),
        module,
        Comp::OCTAVE1_PARAM));
    addLabel(Vec(knobX1 - 10, knobY1 - labelAboveKnob), "Octave");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY1),
        module,
        Comp::FINE1_PARAM));
    addLabel(Vec(knobX2 - 10, knobY1 - labelAboveKnob), "Fine");

 
}

const float jacksX1 = 24;
const float jacksDeltaX = 38;
const float jacksX2 = jacksX1 + 1 * jacksDeltaX;
const float jacksX3 = jacksX1 + 2 * jacksDeltaX;
const float jacksX4 = jacksX1 + 3 * jacksDeltaX;
const float jacksX5 = jacksX1 + 4 * jacksDeltaX;

const float jacksY1 = 276;
const float jacksY2 = jacksY1 + 46;

void SubWidget::addJacks(SubModule *module, std::shared_ptr<IComposite> icomp)
{
    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY1),
        module,
        Comp::VOCT_INPUT));
    addLabel(Vec(jacksX1 - 10, jacksY1 - labelAboveKnob), "V/8");

    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX2, jacksY1),
        module,
        Comp::MAIN_OUTPUT));
    addLabel(Vec(jacksX2 - 10, jacksY1 - labelAboveKnob), "Out");
}



/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SubWidget::SubWidget(SubModule *module)
{
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

     std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addKnobs(module, icomp);
    addJacks(module, icomp);
}

Model *modelSubModule = createModel<SubModule, SubWidget>("squinkylabs-sub");
#endif

