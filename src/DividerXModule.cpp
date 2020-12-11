
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "DividerX.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/ToggleButton.h"

using Comp = DividerX<WidgetComposite>;

/**
 */
struct DividerXModule : Module
{
public:
    DividerXModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void DividerXModule::onSampleRateChange()
{
}

DividerXModule::DividerXModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();
}

void DividerXModule::process(const ProcessArgs& args)
{
    blank->process(args);
}

////////////////////
// module widget
////////////////////

struct DividerXWidget : ModuleWidget
{
    DividerXWidget(DividerXModule *);
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

    void addJacks(DividerXModule *module, std::shared_ptr<IComposite> icomp);
    void addControls(DividerXModule *module, std::shared_ptr<IComposite> icomp);
};

void DividerXWidget::addJacks(DividerXModule *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float jackY = 180;
    const float dy = 46;
     const float labelDy = -15;

    addLabel(
        Vec(jackX - 4, jackY + labelDy),
        "In"
    );
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 0 * dy),
        module,
        Comp::MAIN_INPUT));

    addLabel(
        Vec(jackX - 4, jackY + 1 * dy + labelDy),
        "Out"
    );
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 1 * dy),
        module,
        Comp::FIRST_OUTPUT)); 

    addLabel(
        Vec(jackX - 10, jackY + 2 * dy + labelDy),
        "Stab"
    );
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy),
        module,
        Comp::STABILIZER_OUTPUT)); 

    addLabel(
        Vec(0, jackY + 3 * dy + labelDy),
        "Debug"
    );
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 3 * dy),
        module,
        Comp::DEBUG_OUTPUT)); 
}

void DividerXWidget::addControls(DividerXModule *module, std::shared_ptr<IComposite> icomp)
{
    const float dy = 45;
    const float y = 50;
    const float labelDy = -18;

#if 1
    addLabel(
        Vec(0, y + labelDy),
        "Stab"
    );
    #endif
    ToggleButton* tog = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(11, y),        // was y
        module,  Comp::STABILIZER_PARAM);  
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);
#if 1
    addLabel(
        Vec(11 - 10, y + 1 * dy + labelDy),
        "MinBLEP"
    );
    tog = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(11, y + 1 * dy),
        module,  Comp::MINBLEP_PARAM);  
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);
#endif
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

DividerXWidget::DividerXWidget(DividerXModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/dividerx-panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);
    addControls(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelDividerXModule = createModel<DividerXModule, DividerXWidget>("squinkylabs-dividerx");


