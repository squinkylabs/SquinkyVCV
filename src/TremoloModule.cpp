
#include <sstream>
#include "Squinky.hpp"

#ifdef _TREM
#include "WidgetComposite.h"
#include "Tremolo.h"
#include "ctrl/SqMenuItem.h"

using Comp = Tremolo<WidgetComposite>;

/**
 */
struct TremoloModule : Module
{
public:
    TremoloModule();
    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;
    //Tremolo<WidgetComposite> tremolo;
    std::shared_ptr<Tremolo<WidgetComposite>> tremolo;
private:
};

void TremoloModule::onSampleRateChange()
{
    float rate = SqHelper::engineGetSampleRate();
    tremolo->setSampleRate(rate);
}

#ifdef __V1
TremoloModule::TremoloModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    tremolo = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
#else
TremoloModule::TremoloModule()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS),
    tremolo(std::make_shared<Comp>(this))
{
#endif

    onSampleRateChange();
    tremolo->init();
}

void TremoloModule::step()
{
    tremolo->step();
}

////////////////////
// module widget
////////////////////

struct TremoloWidget : ModuleWidget
{
    TremoloWidget(TremoloModule *);

#ifdef __V1
    void appendContextMenu(Menu *menu) override;
#else
    Menu* createContextMenu() override;
#endif

    void addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
    }
    void addClockSection(TremoloModule *module, std::shared_ptr<IComposite> icomp);
    void addIOSection(TremoloModule *module, std::shared_ptr<IComposite> icomp);
    void addMainSection(TremoloModule *module, std::shared_ptr<IComposite> icomp);
};

#ifdef __V1
void TremoloWidget::appendContextMenu(Menu* theMenu) 
{
    ManualMenuItem* manual = new ManualMenuItem("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/chopper.md");
    theMenu->addChild(manual);   
}
#else
inline Menu* TremoloWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/chopper.md");
    theMenu->addChild(manual);
    return theMenu;
}
#endif

void TremoloWidget::addClockSection(TremoloModule *module, std::shared_ptr<IComposite> icomp)
{
    const float y = 40;        // global offset for clock block
    const float labelY = y + 36;

    //addInput(Port::create<PJ301MPort>(Vec(10, y + 7), Port::INPUT, module, module->tremolo.CLOCK_INPUT));
    addInput(createInput<PJ301MPort>(
        Vec(10, y + 7),
        module, Comp::CLOCK_INPUT));
  
    
    addLabel(Vec(2, labelY), "ckin");

    addParam(SqHelper::createParam<RoundBlackKnob>(
        icomp,
        Vec(110, y),
        module, Comp::LFO_RATE_PARAM));
    addLabel(Vec(104, labelY), "Rate");

    const float cmy = y;
    const float cmx = 60;
    addParam(SqHelper::createParam<RoundBlackSnapKnob>(
        icomp,
        Vec(cmx, cmy),
        module,
        Comp::CLOCK_MULT_PARAM));

    addLabel(Vec(cmx - 8, labelY), "Clock");
    addLabel(Vec(cmx - 19, cmy + 20), "x1");
    addLabel(Vec(cmx + 21, cmy + 20), "int");
    addLabel(Vec(cmx - 24, cmy + 0), "x2");
    addLabel(Vec(cmx + 24, cmy + 0), "x4");
    addLabel(Vec(cmx, cmy - 16), "x3");
}

void TremoloWidget::addIOSection(TremoloModule *module, std::shared_ptr<IComposite> icomp)
{
    const float rowIO = 317;
    const float label = rowIO - 17;
    const float deltaX = 35;
    const float x = 10;

    addInput(createInput<PJ301MPort>(
        Vec(x, rowIO),
        module,
        Comp::AUDIO_INPUT));
    addLabel(Vec(9, label), "in");

    addOutput(createOutput<PJ301MPort>(
        Vec(x + deltaX, rowIO),
        module,
        Comp::AUDIO_OUTPUT));
    addLabel(Vec(x + deltaX - 5, label), "out", SqHelper::COLOR_WHITE);

    addOutput(createOutput<PJ301MPort>(
        Vec(x + 2 * deltaX, rowIO),
        module,
        Comp::SAW_OUTPUT));
    addLabel(Vec(x + 2 * deltaX - 7, label), "saw", SqHelper::COLOR_WHITE);

    addOutput(createOutput<PJ301MPort>(
        Vec(x + 3 * deltaX, rowIO),
        module,
        Comp::LFO_OUTPUT));
    addLabel(Vec(x + 3 * deltaX - 2, label), "lfo", SqHelper::COLOR_WHITE);
}

void TremoloWidget::addMainSection(TremoloModule *module, std::shared_ptr<IComposite> icomp)
{
    const float dn = 3;
    const float knobX = 64;
    const float knobY = 100+dn;
    const float knobDy = 50;
    const float labelX = 100;
    const float labelY = knobY;
    const float trimX = 40;
    const float trimY = knobY + 10;
    const float inY = knobY + 6;
    const float inX = 8;

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(knobX, knobY + 0 * knobDy),
        module,
        Comp::LFO_SHAPE_PARAM));
    
    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(trimX, trimY + 0 * knobDy),
        module,
        Comp::LFO_SHAPE_TRIM_PARAM));
    
    addInput(createInput<PJ301MPort>(
        Vec(inX, inY + 0 * knobDy),
        module,
        Comp::LFO_SHAPE_INPUT));
    addLabel(
        Vec(labelX, labelY + 0 * knobDy), "Shape");

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(knobX, knobY + 1 * knobDy),
        module,
        Comp::LFO_SKEW_PARAM));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(trimX, trimY + 1 * knobDy),
        module,
        Comp::LFO_SKEW_TRIM_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(inX, labelY + 1 * knobDy + 6),
        module,
        Comp::LFO_SKEW_INPUT));
    addLabel(
        Vec(labelX + 1, labelY + 1 * knobDy), "Skew");

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(knobX, knobY + 2 * knobDy),
        module,
        Comp::LFO_PHASE_PARAM));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(trimX, trimY + 2 * knobDy),
        module,
        Comp::LFO_PHASE_TRIM_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(inX, labelY + 2 * knobDy + 6),
        module,
        Comp::LFO_PHASE_INPUT));
    addLabel(
        Vec(labelX, labelY + 2 * knobDy), "Phase");

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(knobX, knobY + 3 * knobDy),
        module,
        Comp::MOD_DEPTH_PARAM));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(trimX, trimY + 3 * knobDy),
        module,
        Comp::MOD_DEPTH_TRIM_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(inX, labelY + 3 * knobDy + 6),
        module,
        Comp::MOD_DEPTH_INPUT));
    addLabel(
        Vec(labelX, labelY + 3 * knobDy), "Depth");
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
TremoloWidget::TremoloWidget(TremoloModule *module)
{   
    setModule(module);
#else
TremoloWidget::TremoloWidget(TremoloModule *module) : ModuleWidget(module)
{
#endif
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/trem_panel.svg");

    addClockSection(module, icomp);
    addMainSection(module, icomp);
    addIOSection(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

#ifdef __V1
Model *modelTremoloModule = createModel<TremoloModule,
    TremoloWidget>("chopper");
#else
Model *modelTremoloModule = Model::create<TremoloModule,
    TremoloWidget>("Squinky Labs",
    "squinkylabs-tremolo",
    "Chopper: Tremolo", EFFECT_TAG, LFO_TAG, CLOCK_MODULATOR_TAG);
#endif
#endif

