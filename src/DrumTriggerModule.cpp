
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _DTMODULE
#include "DrumTrigger.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = DrumTrigger<WidgetComposite>;

/**
 */
struct DrumTriggerModule : Module
{
public:
    DrumTriggerModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> drumTrigger;
private:

};

void DrumTriggerModule::onSampleRateChange()
{
}

DrumTriggerModule::DrumTriggerModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    drumTrigger = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    drumTrigger->init();
}

void DrumTriggerModule::step()
{
    drumTrigger->step();
}

////////////////////
// module widget
////////////////////

struct DrumTriggerWidget : ModuleWidget
{
    DrumTriggerWidget(DrumTriggerModule *);
    DECLARE_MANUAL("Polygate manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/dt.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void makeInputs(DrumTriggerModule* module);
    void makeOutputs(DrumTriggerModule* module);
    void makeOutput(DrumTriggerModule* module, int i);
};

const float width = 90;
const float xLed = width - 51;
const float xJack = width - 20.5;
const float yJack = 277; // 330;
const float dy = 31;
const float yInput = 339;
const float xOff = -10;

void DrumTriggerWidget::makeInputs(DrumTriggerModule* module)
{
    addInput(createInputCentered<PJ301MPort>(
        Vec(31 + xOff, yInput),
        module,
        Comp::CV_INPUT));
   // addLabel(Vec(20.5 + xOff, yInput-30), "CV");
    addInput(createInputCentered<PJ301MPort>(
        Vec(xJack, yInput),
        module,
        Comp::GATE_INPUT));
   // addLabel(Vec(60 + xOff, yInput-30), "Gate");
}

void DrumTriggerWidget::makeOutput(DrumTriggerModule* module, int index)
{
    const float y = yJack -dy * index;
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(xJack, y),
        module,
        index + Comp::GATE0_OUTPUT));

    addChild(createLight<MediumLight<GreenLight>>(
            Vec(xLed, y-4.5),
            module,
            Comp::LIGHT0 + index));
}

void DrumTriggerWidget::makeOutputs(DrumTriggerModule* module)
{
    for (int i=0; i< numTriggerChannels; ++i) {
        makeOutput(module, i);
    }
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

DrumTriggerWidget::DrumTriggerWidget(DrumTriggerModule *module)
{
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/dt_panel.svg");

    makeInputs(module);
    makeOutputs(module);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}
 
Model *modelDrumTriggerModule = createModel<DrumTriggerModule, DrumTriggerWidget>("squinkylabs-dt");
#endif

