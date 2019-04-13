
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIX4

#include "MixerModule.h"
#include "Mix4.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/ToggleButton.h"

#include "ctrl/SqWidgets.h"

using Comp = Mix4<WidgetComposite>;

/**
 */
struct Mix4Module : public MixerModule
{
public:
    Mix4Module();
    /**
     *
     * Overrides of Module functions
     */
    
    void onSampleRateChange() override;

    // Override MixerModule
    void internalProcess() override;

    std::shared_ptr<Comp> Mix4;

protected:
    void setExternalInput(const float*) override;
    void setExternalOutput(float*) override;
private:

};

void Mix4Module::onSampleRateChange()
{
}

void Mix4Module::setExternalInput(const float* buf)
{
    Mix4->setExpansionInputs(buf);
}

void Mix4Module::setExternalOutput(float* buf)
{
    Mix4->setExpansionOutputs(buf);
}

#ifdef __V1
Mix4Module::Mix4Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
Mix4Module::Mix4Module()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS)
{
#endif
    Mix4 = std::make_shared<Comp>(this);
    onSampleRateChange();
    Mix4->init();
}

void Mix4Module::internalProcess()
{
    Mix4->step();
}

////////////////////
// module widget
////////////////////

struct Mix4Widget : ModuleWidget
{
    Mix4Widget(Mix4Module *);
    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void makeStrip(
        Mix4Module*,
        std::shared_ptr<IComposite>,
        int channel,
        std::shared_ptr<ToggleManager>);
};

static const float channelX = 21;           // was 20
static const float dX = 36;             // was 34
static const float channelY = 350;
static const float channelDy = 30;     // just for the bottom jacks
static float volY = 0;
static float muteY = 0;

void Mix4Widget::makeStrip(
    Mix4Module*,
    std::shared_ptr<IComposite> icomp,
    int channel,
    std::shared_ptr<ToggleManager> mgr)
{
    const float x = channelX + channel * dX;

    float y = channelY;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::AUDIO0_INPUT));

    y -= channelDy;
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::CHANNEL0_OUTPUT));

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::LEVEL0_INPUT));

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::PAN0_INPUT));

    y -= channelDy;
    auto mute = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x-12, y-12),
        module,
        channel + Comp::MUTE0_PARAM);
    mute->addSvg("res/square-button-01.svg");
    mute->addSvg("res/square-button-02.svg");
    addParam(mute);
    muteY = y-12;
    
    y -= channelDy;
    auto solo = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x-12, y-12),
        module,
        channel + Comp::SOLO0_PARAM);
    solo->addSvg("res/square-button-01.svg");
    solo->addSvg("res/square-button-02.svg");
    addParam(solo);
    mgr->registerClient(solo);
   
    y -= (channelDy + 2);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::GAIN0_PARAM));
    volY = y;

    y -= (channelDy + 2);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::PAN0_PARAM));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
Mix4Widget::Mix4Widget(Mix4Module *module)
{
    setModule(module);
#else
Mix4Widget::Mix4Widget(Mix4Module *module) : ModuleWidget(module)
{
#endif
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/mix4_panel.svg");
     std::shared_ptr<IComposite> icomp = Comp::getDescription();

    std::shared_ptr<ToggleManager> mgr = std::make_shared<ToggleManager>();
    for (int i=0; i< Comp::numChannels; ++i) {
        makeStrip(module, icomp, i, mgr);
    }

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1
Model *modelMix4Module = createModel<Mix4Module, Mix4Widget>("squinkylabs-mix4");
#else
Model *modelMix4Module = Model::create<Mix4Module,
    Mix4Widget>("Squinky Labs",
    "squinkylabs-mix4",
    "-- Mix4 --", RANDOM_TAG);
#endif
#endif

