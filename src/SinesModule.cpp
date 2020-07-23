
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SINES
#include "Sines.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Sines<WidgetComposite>;

/**
 */
struct SinesModule : Module
{
public:
    SinesModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void SinesModule::onSampleRateChange()
{
}

SinesModule::SinesModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();
}

void SinesModule::process(const ProcessArgs& args)
{
    blank->process(args);
}

////////////////////
// module widget
////////////////////

struct SinesWidget : ModuleWidget
{
    SinesWidget(SinesModule *);
    DECLARE_MANUAL("Blank Manul", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }


    void addJacks(SinesModule *module, std::shared_ptr<IComposite> icomp);
};


void SinesWidget::addJacks(SinesModule *module, std::shared_ptr<IComposite> icomp)
{
    addInput(createInput<PJ301MPort>(
        Vec(50, 340),
        module,
        Comp::V_OCT_INPUT));
    addLabel( Vec(44, 320), "V/Oct");

    addInput(createInput<PJ301MPort>(
        Vec(90, 340),
        module,
        Comp::GATE_INPUT));
    addLabel( Vec(84, 320), "Gate");

    addOutput(createOutput<PJ301MPort>(
        Vec(130, 340),
        module,
        Comp::V_OCT_INPUT));
    addLabel( Vec(124, 320), "Out");
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SinesWidget::SinesWidget(SinesModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelSinesModule = createModel<SinesModule, SinesWidget>("squinkylabs-sines");
#endif

