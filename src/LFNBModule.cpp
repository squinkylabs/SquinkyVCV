

#include "Squinky.hpp"
#ifdef _LFN
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqWidgets.h"
#include "WidgetComposite.h"
#include "LFNB.h"

#include <sstream>

using Comp = LFNB<WidgetComposite>;

/**
 */
struct LFNBModule : public Module
{
public:
    LFNBModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Comp lfn;
private:

};

void LFNBModule::onSampleRateChange()
{
    lfn.setSampleTime(SqHelper::engineGetSampleTime());
}

#ifdef __V1
LFNBModule::LFNBModule() : lfn(this)
{
    config(lfn.NUM_PARAMS,lfn.NUM_INPUTS,lfn.NUM_OUTPUTS,lfn.NUM_LIGHTS);
    onSampleRateChange();
    lfn.init();
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
}
#else
LFNBModule::LFNBModule()
    : Module(lfn.NUM_PARAMS,
    lfn.NUM_INPUTS,
    lfn.NUM_OUTPUTS,
    lfn.NUM_LIGHTS),
    lfn(this)
{
    onSampleRateChange();
    lfn.init();
}
#endif

void LFNBModule::step()
{
    lfn.step();
}

////////////////////
// module widget
////////////////////

/**
 * This class updates the base frequencies of
 * all the labels when the master changes
 */
class LFNBLabelUpdater
{
public:
    void update(struct LFNBWidget& widget);
    void makeLabel(struct LFNBWidget& widget, int index, float x, float y);
private:
    Label*  labels[5] = {0,0,0,0,0};
    float baseFrequency = -1;
};

struct LFNBWidget : ModuleWidget
{
    LFNBWidget(LFNBModule *);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();


    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
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
        updater.update(*this);
        if (module) {
            module->lfn.pollForChangeOnUIThread();
        } 
        ModuleWidget::step();
    }

#ifdef __V1
    void appendContextMenu(Menu *menu) override;
#else
    Menu* createContextMenu() override;
#endif

    void addStage(int i);

    LFNBLabelUpdater updater;
    // note that module will be null in some cases
    LFNBModule* module;

    ParamWidget* xlfnWidget = nullptr;
};

static const float knobX = 42;
static const float knobY = 100;
static const float knobDy = 50;
static const float inputY = knobY + 16;
static const float inputX = 6;
static const float labelX = 2;

#if 0
void LFNBWidget::addStage(int index)
{
    // make a temporary one for instantiation controls,
    // in case module is null.
    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(knobX, knobY + index * knobDy),
        module, Comp::EQ0_PARAM + index));

    updater.makeLabel((*this), index, labelX, knobY - 2 + index * knobDy);

    addInput(createInput<PJ301MPort>(
        Vec(inputX, inputY + index * knobDy),
        module, Comp::EQ0_INPUT + index));
}
#endif

#ifdef __V1
void LFNBWidget::appendContextMenu(Menu* theMenu) 
{
    ManualMenuItem* manual = new ManualMenuItem("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/lfn.md");
    theMenu->addChild(manual);
    
    MenuLabel *spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    SqMenuItem_BooleanParam * item = new SqMenuItem_BooleanParam(
        xlfnWidget);
    item->text = "Extra Low Frequency";
    theMenu->addChild(item);
}
#else
inline Menu* LFNBWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();

    ManualMenuItem* manual = new ManualMenuItem("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/lfn.md");
    theMenu->addChild(manual);
    
    MenuLabel *spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    SqMenuItem_BooleanParam * item = new SqMenuItem_BooleanParam(
        xlfnWidget);
    item->text = "Extra Low Frequency";
    theMenu->addChild(item);
    return theMenu;
}
#endif

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
LFNBWidget::LFNBWidget(LFNBModule *module) : module(module)
{
    setModule(module);
#else
LFNBWidget::LFNBWidget(LFNBModule *module) : ModuleWidget(module), module(module)
{
#endif
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");


#if 0
    addOutput(createOutput<PJ301MPort>(
        Vec(59, inputY - knobDy - 1),
        module,
        Comp::OUTPUT));
    addLabel(
        Vec(54, inputY - knobDy - 18), "out", SqHelper::COLOR_WHITE);

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(10, knobY - 1 * knobDy),
        module,
        Comp::FREQ_RANGE_PARAM));

    for (int i = 0; i < 5; ++i) {
        addStage(i);
    }
#endif

    xlfnWidget = SqHelper::createParam<NullWidget>(
        icomp,
        Vec(0, 0),
        module,
        Comp::XLFNB_PARAM);
    xlfnWidget->box.size.x = 0;
    xlfnWidget->box.size.y = 0;
    addParam(xlfnWidget);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void LFNBLabelUpdater::makeLabel(struct LFNBWidget& widget, int index, float x, float y)
{
    labels[index] = widget.addLabel(Vec(x, y), "Hz");
}

void LFNBLabelUpdater::update(struct LFNBWidget& widget)
{
    // needs update for b

    // This will happen often
    if (!widget.module) {
        return;
    }
    float baseFreq = widget.module->lfn.getBaseFrequency();
    const bool isXLFN = widget.module->lfn.isXLFN();
    const float moveLeft = isXLFN ? 3 : 0;
    const int digits = isXLFN ? 2 : 1;
    if (baseFreq != baseFrequency) {
        baseFrequency = baseFreq;
        for (int i = 0; i < 5; ++i) {
            if (labels[i] == nullptr) {
                return;
            }
            std::stringstream str;
            str.precision(digits);
            str.setf(std::ios::fixed, std::ios::floatfield);
            str << baseFreq;
            labels[i]->text = str.str();
            labels[i]->box.pos.x = labelX - moveLeft;
            baseFreq *= 2.0f;
        }
    }
}

#ifndef __V1
Model *modelLFNBModule = Model::create<LFNBModule,
    LFNBWidget>("Squinky Labs",
    "squinkylabs-lfnb",

    "LFNB: Random Voltages", NOISE_TAG, RANDOM_TAG, LFO_TAG);
#else
Model *modelLFNBModule = createModel<LFNBModule, LFNBWidget>(
    "squinkylabs-lfnb");
#endif

#endif