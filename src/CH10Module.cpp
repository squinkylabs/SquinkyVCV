#include "Squinky.hpp"

#ifdef _CH10
#include "ctrl/SqWidgets.h"
#include "WidgetComposite.h"
#include "CH10.h"
#include "ctrl/ToggleButton.h"

#include <sstream>

using Comp = CH10<WidgetComposite>;

/**
 */
struct CH10Module : Module
{
public:
    CH10Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> ch10;

private:

};

void CH10Module::onSampleRateChange()
{
}


#ifdef __V1x
CH10Module::CH10Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
CH10Module::CH10Module()
    : Module(CH10<WidgetComposite>::NUM_PARAMS,
    CH10<WidgetComposite>::NUM_INPUTS,
    CH10<WidgetComposite>::NUM_OUTPUTS,
    CH10<WidgetComposite>::NUM_LIGHTS)
{
#endif
    ch10 = std::make_shared<Comp>(this);
    onSampleRateChange();
    ch10->init();
}

void CH10Module::step()
{
    ch10->step();
}

////////////////////
// module widget
////////////////////

struct CH10Widget : ModuleWidget
{
    CH10Widget(CH10Module *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void makeA(CH10Module *, std::shared_ptr<IComposite> icomp);
    void makeB(CH10Module *, std::shared_ptr<IComposite> icomp);
    void makeAB(CH10Module *, std::shared_ptr<IComposite> icomp);
    void addSwitch(float x, float y, int id, std::shared_ptr<IComposite> icomp);
    void makeVCO(CH10Module*, int whichOne, std::shared_ptr<IComposite> icomp);
};

const static float gridSize = 28;
const static float gridCol1 = 140;
const static float gridRow1 = 300;

inline void CH10Widget::makeA(CH10Module *, std::shared_ptr<IComposite> icomp)
{
    for (int i = 0; i < 10; ++i) {
        const float x = gridCol1;
        const float y = gridRow1 - i * gridSize;
        addSwitch(x, y, CH10<Widget>::A0_PARAM + i, icomp);
    }
}

inline void CH10Widget::makeB(CH10Module *, std::shared_ptr<IComposite> icomp)
{
    for (int i = 0; i < 10; ++i) {
        const float x = gridCol1 + gridSize * (i + 1);
        const float y = gridRow1 + gridSize;
        addSwitch(x, y, CH10<Widget>::B0_PARAM + i, icomp);
    }
}

inline void CH10Widget::makeAB(CH10Module *, std::shared_ptr<IComposite> icomp)
{
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            float x = gridCol1 + gridSize * (col + 1);
            float y = gridRow1 - row * gridSize;
            int id = CH10<Widget>::A0B0_PARAM +
                col + row * 10;
            addSwitch(x, y, id, icomp);
        }
    }
}

inline void CH10Widget::addSwitch(float x, float y, int id, std::shared_ptr<IComposite> icomp)
{
    ToggleButton* tog = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x, y),
        module,
        id);

    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);
}

const float rowSpacing = 40;
const float vcoACol = 50;
const float vcoBCol = 90;
const float vcoOctRow = 60;
const float vcoSemiRow = vcoOctRow + rowSpacing;
const float vcoCVRow = vcoSemiRow + rowSpacing;

inline void CH10Widget::makeVCO(CH10Module* module, int whichVCO, std::shared_ptr<IComposite> icomp)
{
    const float x = whichVCO ? vcoBCol : vcoACol;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, vcoOctRow),
        module,
        Comp::AOCTAVE_PARAM + whichVCO));

    addParam(SqHelper::createParamCentered<Blue30SnapKnob>(
        icomp,
        Vec(x, vcoSemiRow), module,
        CH10<WidgetComposite>::ASEMI_PARAM + whichVCO));

    addInput(createInputCentered<PJ301MPort>(

        Vec(x, vcoCVRow),
        module,
        CH10<WidgetComposite>::ACV_INPUT + whichVCO));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1x
CH10Widget::CH10Widget(CH10Module *module)
{
    setModule(module);
#else
CH10Widget::CH10Widget(CH10Module *module) : ModuleWidget(module)
{
#endif
    box.size = Vec(35 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/ch10_panel.svg");
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    makeA(module, icomp);
    makeB(module, icomp);
    makeAB(module, icomp);
    makeVCO(module, 0, icomp);
    makeVCO(module, 1, icomp);

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(70, 300),
        module,
        CH10<WidgetComposite>::MIXED_OUTPUT));

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

#ifdef __V1x
Model *modelCH10Module = createModel<CH10Module, CH10Widget>("squinkylabs-ch10");
#else
Model *modelCH10Module = Model::create<CH10Module,
    CH10Widget>("Squinky Labs",
    "squinkylabs-ch10",
    "-- ch10 --", RANDOM_TAG);
#endif
#endif

