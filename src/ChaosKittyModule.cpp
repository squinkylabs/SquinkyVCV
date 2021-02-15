
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _CHAOS
#include "ChaosKitty.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidget.h"

using Comp = ChaosKitty<WidgetComposite>;

/**
 */
struct ChaosKittyModule : Module
{
public:
    ChaosKittyModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> comp;
private:

};

void ChaosKittyModule::onSampleRateChange()
{
    INFO("ChaosKittyModule::onSampleRateChange");
    comp->onSampleRateChange(SqHelper::engineGetSampleRate(), SqHelper::engineGetSampleTime());
}

ChaosKittyModule::ChaosKittyModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    comp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    INFO("ChaosKittyModule::ChaosKittyModule( will call onSamplRateChange");
    onSampleRateChange();
    comp->init();
}

void ChaosKittyModule::step()
{
    comp->step();
}

////////////////////
// module widget
////////////////////

struct ChaosKittyWidget : ModuleWidget
{
    ChaosKittyWidget(ChaosKittyModule *);
    DECLARE_MANUAL("Chaos Kitty manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/chaos-kitty.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

ChaosKittyWidget::ChaosKittyWidget(ChaosKittyModule *module)
{
    setModule(module);
    // box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    addLabel(Vec(60, 14), "Chaos Kitty");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    const float xInput = 40;
    const float xTrim = 80;
    const float xParam = 140;

    // row 1: chaos

    const float dy = 60;
    const float yRow1 = 140;
    const float yRow2 = yRow1 + dy;
    const float yRow3 = yRow1 + 2 * dy;

    addLabel( Vec(xTrim-30, yRow1 - 40), "Chaos" );
    addInput(createInputCentered<PJ301MPort>(
        Vec(xInput, yRow1),
        module,
        Comp::CHAOS_INPUT));

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(xParam, yRow1),
        module, 
        Comp::CHAOS_PARAM));

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(xParam, yRow2),
        module, 
        Comp::BRIGHTNESS_PARAM));

      addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(xParam, yRow3),
        module, 
        Comp::RESONANCE_PARAM));

    auto octaveControl = SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(40, 240),
        module, 
        Comp::OCTAVE_PARAM);
    octaveControl->snap = true;
    octaveControl->smooth = false;
    addParam(octaveControl);

    auto p = SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(xTrim, yRow1),
        module,
        Comp::CHAOS_TRIM_PARAM);
    addParam(p);

    // *************************************************

    PopupMenuParamWidget* popup = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(50, 60),
        module,
        Comp::TYPE_PARAM);
    popup->box.size.x = 76;    // width
    popup->box.size.y = 22;     // should set auto like button does
    popup->text = "noise";
    popup->setLabels(Comp::typeLabels());
    addParam(popup);


    // *************************************************

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(140, 340),
        module,
        Comp::MAIN_OUTPUT));
    addLabel( Vec(130, 310), "Out");
    
    addInput(createInputCentered<PJ301MPort>(
        Vec(40, 340),
        module,
        Comp::V8_INPUT));
    addLabel( Vec(40, 310), "V/8 in");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelChaosKittyModule = createModel<ChaosKittyModule, ChaosKittyWidget>("squinkylabs-chaoskitty");
#endif

