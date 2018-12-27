
#include "ctrl/SqMenuItem.h"
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#include "ctrl/ToggleButton.h"
#include "SqWidgets.h"
#include "LFN.h"


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

    LFN<WidgetComposite> lfn;
private:

};

void LFNModule::onSampleRateChange()
{
    lfn.setSampleTime(engineGetSampleTime());
}

LFNModule::LFNModule()
    : Module(lfn.NUM_PARAMS,
    lfn.NUM_INPUTS,
    lfn.NUM_OUTPUTS,
    lfn.NUM_LIGHTS),
    lfn(this)
{
    onSampleRateChange();
    lfn.init();
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
    LFNModule * module;
    Label*  labels[5] = {0,0,0,0,0};
    float baseFrequency = -1;
};

struct LFNWidget : ModuleWidget
{
    LFNWidget(LFNModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    // why not on step()?
   // void draw(NVGcontext *vg) override
    void step() override
    {
        updater.update(*this);
        module.lfn.pollForChangeOnUIThread();
       // ModuleWidget::draw(vg);
        ModuleWidget::step();
    }

    Menu* createContextMenu() override;

    void addStage(int i);

    LFNLabelUpdater updater;
    LFNModule&     module;
    ParamWidget* xlfnWidget = nullptr;
};

static const float knobX = 42;
static const float knobY = 100;
static const float knobDy = 50;
static const float inputY = knobY + 16;
static const float inputX = 6;
static const float labelX = 2;

void LFNWidget::addStage(int index)
{
    const float gmin = -5;
    const float gmax = 5;
    const float gdef = 0;
    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(knobX, knobY + index * knobDy),
        &module, module.lfn.EQ0_PARAM + index, gmin, gmax, gdef));

    updater.makeLabel((*this), index, labelX, knobY - 2 + index * knobDy);

    addInput(Port::create<PJ301MPort>(Vec(inputX, inputY + index * knobDy),
        Port::INPUT, &module, module.lfn.EQ0_INPUT + index));
}

inline Menu* LFNWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    MenuLabel *spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    SqMenuItem_BooleanParam * item = new SqMenuItem_BooleanParam(
        xlfnWidget);
    item->text = "Extra Low Frequency";
    theMenu->addChild(item);
    return theMenu;
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
LFNWidget::LFNWidget(LFNModule *module) : ModuleWidget(module), module(*module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/lfn_panel.svg")));
        addChild(panel);
    }

    addOutput(Port::create<PJ301MPort>(
        Vec(59, inputY - knobDy - 1), Port::OUTPUT, module, module->lfn.OUTPUT));
    addLabel(
        Vec(54, inputY - knobDy - 18), "out", COLOR_WHITE);

    addParam(ParamWidget::create<Rogan1PSBlue>(
        Vec(10, knobY - 1 * knobDy), module, module->lfn.FREQ_RANGE_PARAM, -5, 5, 0));

   // addLabel(Vec(59, knobY - 1 * knobDy), "R");

    for (int i = 0; i < 5; ++i) {
        addStage(i);
    }

    xlfnWidget = ParamWidget::create<NullWidget>(
        Vec(0, 0), module, module->lfn.XLFN_PARAM, 0, 1, 0);
    xlfnWidget->box.size.x = 0;
    xlfnWidget->box.size.y = 0;
    addParam(xlfnWidget);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void LFNLabelUpdater::makeLabel(struct LFNWidget& widget, int index, float x, float y)
{
    labels[index] = widget.addLabel(Vec(x, y), "Hz");
}

void LFNLabelUpdater::update(struct LFNWidget& widget)
{
    float baseFreq = widget.module.lfn.getBaseFrequency();
    const bool isXLFN = widget.module.lfn.isXLFN();
    const float moveLeft = isXLFN ? 3 : 0;
    const int digits = isXLFN ? 2 : 1;
    if (baseFreq != baseFrequency) {
        baseFrequency = baseFreq;
        for (int i = 0; i < 5; ++i) {
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

Model *modelLFNModule = Model::create<LFNModule,
    LFNWidget>("Squinky Labs",
    "squinkylabs-lfn",
    "LFN: Random Voltages", NOISE_TAG, RANDOM_TAG, LFO_TAG);

