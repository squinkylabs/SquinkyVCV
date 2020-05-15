
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _WVCO
#include "WVCO.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidgetv1.h"


using Comp = WVCO<WidgetComposite>;

/**
 */
struct WVCOModule : Module
{
public:
    WVCOModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void WVCOModule::onSampleRateChange()
{
}

WVCOModule::WVCOModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();
}

void WVCOModule::step()
{
    blank->step();
}

////////////////////
// module widget
////////////////////

struct WVCOWidget : ModuleWidget
{
    WVCOWidget(WVCOModule *);
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

    void addKnobs(WVCOModule *module, std::shared_ptr<IComposite> icomp);
    void addJacks(WVCOModule *module, std::shared_ptr<IComposite> icomp);
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

void WVCOWidget::addKnobs(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    // first row
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX1, knobY1),
        module,
        Comp::OCTAVE_PARAM));
    addLabel(Vec(knobX1 - 10, knobY1 - labelAboveKnob), "Octave");

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX2, knobY1),
        module,
        Comp::FREQUENCY_MULTIPLIER_PARAM));
    addLabel(Vec(knobX2 - 10, knobY1 - labelAboveKnob), "Ratio");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3, knobY1),
        module,
        Comp::FINE_TUNE_PARAM));
    addLabel(Vec(knobX3 - 10, knobY1 - labelAboveKnob), "Fine");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX4, knobY1),
        module,
        Comp::FM_DEPTH_PARAM));
    addLabel(Vec(knobX4 - 10, knobY1 - labelAboveKnob), "Mod");

    // second row
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX1, knobY2),
        module,
        Comp::LINEAR_FM_DEPTH_PARAM));
    addLabel(Vec(knobX1 - 10, knobY2 - labelAboveKnob), "LFM-0");

#if 1
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX2, knobY2),
        module,
        Comp::WAVE_SHAPE_PARAM));
    addLabel(Vec(knobX2 - 10, knobY2 - labelAboveKnob), "Wvfm");

#else
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX2, knobY2),
        module,
        Comp::WAVE_SHAPE_PARAM);
    p->box.size.x = 50;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->text = "Sine";
    p->setLabels(Comp::getWaveformNames());
    addParam(p);
#endif

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3, knobY2),
        module,
        Comp::WAVESHAPE_GAIN_PARAM));
    addLabel(Vec(knobX3 - 10, knobY2 - labelAboveKnob), "Shape");

    // third row
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX4, knobY2),
        module,
        Comp::FEEDBACK_PARAM));
    addLabel(Vec(knobX4 - 10, knobY2 - labelAboveKnob), "Fbck");

}

const float jacksX1 = 24;
const float jacksDeltaX = 38;
const float jacksX2 = jacksX1 + 1 * jacksDeltaX;
const float jacksX3 = jacksX1 + 2 * jacksDeltaX;
const float jacksX4 = jacksX1 + 3 * jacksDeltaX;
const float jacksX5 = jacksX1 + 4 * jacksDeltaX;

const float jacksY1 = 260;
const float jacksY2 = jacksY1 + 46;
#if 0
  FM_INPUT,
        LINEAR_FM_INPUT,
        GATE_INPUT,
        #endif

void WVCOWidget::addJacks(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY1),
        module,
        Comp::VOCT_INPUT));
    addLabel(Vec(jacksX1 - 10, jacksY1 - labelAboveKnob), "V/8");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX2, jacksY1),
        module,
        Comp::FM_INPUT));
    addLabel(Vec(jacksX2 - 10, jacksY1 - labelAboveKnob), "Mod");

     addInput(createInput<PJ301MPort>(
        Vec(jacksX3, jacksY1),
        module,
        Comp::LINEAR_FM_INPUT));
    addLabel(Vec(jacksX3 - 10, jacksY1 - labelAboveKnob), "LFM-0");

     addInput(createInput<PJ301MPort>(
        Vec(jacksX4, jacksY1),
        module,
        Comp::GATE_INPUT));
    addLabel(Vec(jacksX4 - 10, jacksY1 - labelAboveKnob), "Gate");
    
    // second row
    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX1, jacksY2),
        module,
        Comp::MAIN_OUTPUT));
    addLabel(Vec(jacksX1 - 10, jacksY2 - labelAboveKnob), "Out");
}




/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

WVCOWidget::WVCOWidget(WVCOModule *module)
{
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/wvco_panel.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addKnobs(module, icomp);
    addJacks(module, icomp);

}

Model *modelWVCOModule = createModel<WVCOModule, WVCOWidget>("squinkylabs-wvco");
#endif

