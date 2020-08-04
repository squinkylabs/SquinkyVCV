
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SINES
#include "Sines.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

using Comp = Sines<WidgetComposite>;

#define _DRAWBAR

#ifdef _DRAWBAR
class Drawbar : public app::SvgSlider {
public:
    Drawbar(const std::string& handleName) {
        WARN("loading drawbar svg");
        math::Vec margin = math::Vec(3.5, 3.5);

        maxHandlePos = math::Vec(-7, 10).plus(margin);
		minHandlePos = math::Vec(-7, 90).plus(margin);

        setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scaletx.svg")));
		this->setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, handleName.c_str())));
        background->box.pos = margin;
        this->box.size.x = 29;
        this->box.size.y = 120;
      
	}
};
#endif

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
    printf("CALLING INIT\n"); fflush(stdout);
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
    DECLARE_MANUAL("Organ Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/og/docs/og.md");

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
    void addDrawbars(SinesModule *module, std::shared_ptr<IComposite> icomp);
    void addOtherControls(SinesModule *module, std::shared_ptr<IComposite> icomp);
};

template <typename TLightBase = RedLight>
struct LEDLightSliderFixed : LEDLightSlider<TLightBase> {
	LEDLightSliderFixed() {
		//this->setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LEDSliderHandle.svg")));
        this->setHandleSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/LEDSliderHandle.svg")));
	}
};

static float topRow = 81;
void SinesWidget::addOtherControls(SinesModule *module, std::shared_ptr<IComposite> icomp)
{
    const float col = 163;
   // const float row = 81;
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(col, topRow + 5),
        module,
       Comp::DECAY_PARAM));
    auto l = addLabel(Vec(col - 34, topRow ), "fast");
    l->fontSize = 11;
    l = addLabel(Vec(col - 34, topRow + 10), "slow");
    l->fontSize = 11;

    float keyclickX = 75;
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(keyclickX, topRow + 5),
        module,
        Comp::KEYCLICK_PARAM));
    addLabel(Vec(keyclickX - 34, topRow), "click");
}


const char* handles[] = {
    "res/blue-handle-16.svg",
    "res/blue-handle-513.svg",
    "res/white-handle-8.svg",
    "res/white-handle-4.svg",
    "res/black-handle-223.svg",
    "res/white-handle-2.svg",
    "res/black-handle-135.svg",
    "res/black-handle-113.svg",
    "res/white-handle-1.svg"
};
void SinesWidget::addDrawbars(SinesModule *module, std::shared_ptr<IComposite> icomp)
{
    float drawbarX = 7;
    float drawbarDX = 29;
    //float drawbarY = 190;
    float drawbarY = 136;
  //  float row1 = 81;

    for (int i = 0; i < 9; ++i) {
        std::string handleName = handles[i];
        auto drawbar = new Drawbar(handleName);
        float x = drawbarX + i * drawbarDX;
        drawbar->box.pos = Vec(x, drawbarY);
        if (module) {
            drawbar->paramQuantity = module->paramQuantities[Comp::DRAWBAR1_PARAM +i];
        }
        addParam(drawbar);

        
        addInput(createInput<PJ301MPort>(
            Vec(x, 261),
            module,
            Comp::DRAWBAR1_INPUT + i));

    }
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(190, topRow),
        module,  Comp::PERCUSSION1_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(233, topRow),
        module,  Comp::PERCUSSION2_PARAM));

}

void SinesWidget::addJacks(SinesModule *module, std::shared_ptr<IComposite> icomp)
{
    addInput(createInput<PJ301MPort>(
        Vec(107, 322),
        module,
        Comp::VOCT_INPUT));
    addLabel( Vec(107, 304), "V/Oct");

    addInput(createInput<PJ301MPort>(
        Vec(167, 322),
        module,
        Comp::GATE_INPUT));
    addLabel( Vec(167, 304), "Gate");

    addOutput(createOutput<PJ301MPort>(
        Vec(225, 322),
        module,
        Comp::MAIN_OUTPUT));
    addLabel( Vec(225, 304), "Out");
#if 0
    addOutput(createOutput<PJ301MPort>(
        Vec(160, 340),
        module,
        Comp::DEBUG_OUTPUT));
    addLabel( Vec(160, 320), "Debug");
#endif
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(11, 319),
        module,  Comp::ATTACK_PARAM));
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(55, 319),
        module,  Comp::RELEASE_PARAM));
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
    addDrawbars(module, icomp);
    addOtherControls(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelSinesModule = createModel<SinesModule, SinesWidget>("squinkylabs-sines");
#endif

