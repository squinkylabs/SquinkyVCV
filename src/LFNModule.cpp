

#include "Squinky.hpp"
#ifdef _LFN
#include "DrawTimer.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqWidgets.h"
#include "WidgetComposite.h"
#include "LFN.h"

#include "SqStream.h"

#ifdef _TIME_DRAWING
static DrawTimer drawTimer("LFN");
#endif

using Comp = LFN<WidgetComposite>;

/**
 */
struct LFNModule : public Module
{
public:
    LFNModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Comp lfn;
private:

};

void LFNModule::onSampleRateChange()
{
    lfn.setSampleTime(SqHelper::engineGetSampleTime());
}

LFNModule::LFNModule() : lfn(this)
{
    config(lfn.NUM_PARAMS,lfn.NUM_INPUTS,lfn.NUM_OUTPUTS,lfn.NUM_LIGHTS);
    onSampleRateChange();
    lfn.init();
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
}

void LFNModule::step()
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
class LFNLabelUpdater
{
public:
    void update(struct LFNWidget& widget);
    void makeLabel(struct LFNWidget& widget, int index, float x, float y);
private:
    Label*  labels[5] = {0,0,0,0,0};
    float baseFrequency = -1;
};

struct LFNWidget : ModuleWidget
{
    LFNWidget(LFNModule *);
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

    void appendContextMenu(Menu *menu) override;
    void addStage(int i);

    LFNLabelUpdater updater;
    // note that module will be null in some cases
    LFNModule* module;

    ParamWidget* xlfnWidget = nullptr;

#ifdef _TIME_DRAWING
    // LFN: avg = 30.989896, stddev = 10.567962 (us) Quota frac=0.185939
    void draw(const DrawArgs &args) override
    {
        DrawLocker l(drawTimer);
        ModuleWidget::draw(args);
    }
#endif
};

static const float knobX = 42;
static const float knobY = 100;
static const float knobDy = 50;
static const float inputY = knobY + 16;
static const float inputX = 6;
static const float labelX = 2;

void LFNWidget::addStage(int index)
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

void LFNWidget::appendContextMenu(Menu* theMenu) 
{
    MenuLabel *spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    ManualMenuItem* manual = new ManualMenuItem("LFN manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/lfn.md");
    theMenu->addChild(manual);
    

    SqMenuItem_BooleanParam * item = new SqMenuItem_BooleanParam(
        xlfnWidget);
    item->text = "Extra Low Frequency";
    theMenu->addChild(item);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

LFNWidget::LFNWidget(LFNModule *module) : module(module)
{
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/lfn_panel.svg");

    addOutput(createOutput<PJ301MPort>(
        Vec(59, inputY - knobDy - 1),
        module,
        LFN<WidgetComposite>::OUTPUT));
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

    xlfnWidget = SqHelper::createParam<NullWidget>(
        icomp,
        Vec(0, 0),
        module,
        Comp::XLFN_PARAM);
    xlfnWidget->box.size.x = 0;
    xlfnWidget->box.size.y = 0;
    addParam(xlfnWidget);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void LFNLabelUpdater::makeLabel(struct LFNWidget& widget, int index, float x, float y)
{
    labels[index] = widget.addLabel(Vec(x, y), "Hz");
}

void LFNLabelUpdater::update(struct LFNWidget& widget)
{
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
            SqStream str;
            str.precision(digits);
            str.add(baseFreq);
            labels[i]->text = str.str();
            labels[i]->box.pos.x = labelX - moveLeft;
            baseFreq *= 2.0f;
        }
    }
}

Model *modelLFNModule = createModel<LFNModule, LFNWidget>(
    "squinkylabs-lfn");

#endif