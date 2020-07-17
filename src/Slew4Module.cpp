
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SLEW
#include "DrawTimer.h"
#include "Slew4.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

#ifdef _TIME_DRAWING
static DrawTimer drawTimer("Slade");
#endif

using Comp = Slew4<WidgetComposite>;

/**
 */
struct Slew4Module : Module
{
public:
    Slew4Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> slew;
private:

};

void Slew4Module::onSampleRateChange()
{
    slew->onSampleRateChange();
}

Slew4Module::Slew4Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    slew = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    slew->init();
}

void Slew4Module::step()
{
    slew->step();
}

////////////////////
// module widget
////////////////////

struct Slew4Widget : ModuleWidget
{
    Slew4Widget(Slew4Module *);
    DECLARE_MANUAL("Slade manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/slew4.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(Slew4Module *);
    void addScrews();
    void addOther(Slew4Module*, std::shared_ptr<IComposite> icomp);


#ifdef _TIME_DRAWING
    // Slade: avg = 71.970501, stddev = 16.551967 (us) Quota frac=0.431823
    void draw(const DrawArgs &args) override
    {
        DrawLocker l(drawTimer);
        ModuleWidget::draw(args);
    }
#endif
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
Slew4Widget::Slew4Widget(Slew4Module *module)
{
    setModule(module);
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/slew_panel.svg");

    addJacks(module);
    addScrews();
    addOther(module, icomp);
}

float jackY = 50;       // was 60
float jackDy = 30;
float jackX = 18;
float jackDx = 28;
const float jackLabelDy = -34;

void Slew4Widget::addJacks(Slew4Module *module)
{
    for (int i=0; i<8; ++i) {
        addInput(createInputCentered<PJ301MPort>(
            Vec(jackX, jackY + i * jackDy),
            module,
            Comp::INPUT_TRIGGER0 + i));

        addInput(createInputCentered<PJ301MPort>(
            Vec(jackX + jackDx, jackY + i * jackDy),
            module,
            Comp::INPUT_AUDIO0 + i));

        addOutput(createOutputCentered<PJ301MPort>(
            Vec(jackX + 2 * jackDx, jackY + i * jackDy),
            module,
            Comp::OUTPUT0 + i));
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(jackX + 3 * jackDx, jackY + i * jackDy),
            module,
            Comp::OUTPUT_MIX0 + i));
    }
    addLabel(Vec(jackX -22, jackY+jackLabelDy), "Gate");
    addLabel(Vec(jackX + jackDx - 17, jackY+jackLabelDy), "(In)");
    addLabel(Vec(jackX + 2 * jackDx - 18, jackY+jackLabelDy), "Out");
    addLabel(Vec(jackX + 3 * jackDx - 18, jackY+jackLabelDy), "Mix");
}

static const float knobY= 310;
static const float knobX = 20;
static const float knobDx = 36;
static const float labelAboveKnob = 36;
static const float jackY2 = 342;

void Slew4Widget::addOther(Slew4Module*, std::shared_ptr<IComposite> icomp)
{
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(knobX, knobY),
        module,
        Comp::PARAM_RISE));
    addLabel(Vec(knobX - 20, knobY - labelAboveKnob), "Rise");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(knobX + knobDx, knobY),
        module,
        Comp::PARAM_FALL));
    addLabel(Vec(knobX + 2 + knobDx - 20, knobY - labelAboveKnob), "Fall");

     addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(knobX + 2 * knobDx, knobY),
        module,
        Comp::PARAM_LEVEL));
    addLabel(Vec(knobX - 3 + 2 * knobDx - 20, knobY - labelAboveKnob), "Level");

    addInput(createInputCentered<PJ301MPort>(
         Vec(knobX, jackY2),
         module,
         Comp::INPUT_RISE));
    addInput(createInputCentered<PJ301MPort>(
         Vec(knobX + knobDx, jackY2),
         module,
         Comp::INPUT_FALL));
};

void Slew4Widget::addScrews()
{
    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelSlew4Module = createModel<Slew4Module, Slew4Widget>("squinkylabs-slew4");
#endif

