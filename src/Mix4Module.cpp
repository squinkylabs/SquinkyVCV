
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIX4

#include "MixerModule.h"
#include "Mix4.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/ToggleButton.h"
#include "ctrl/SqToggleLED.h"


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
    void requestModuleSolo(SoloCommands) override;

protected:
    void setExternalInput(const float*) override;
    void setExternalOutput(float*) override;
private:
    std::shared_ptr<Comp> Mix4;

};

void Mix4Module::onSampleRateChange()
{
    Mix4->onSampleRateChange();
}

void Mix4Module::setExternalInput(const float* buf)
{
    Mix4->setExpansionInputs(buf);
}

void Mix4Module::setExternalOutput(float* buf)
{
    Mix4->setExpansionOutputs(buf);
}

void Mix4Module::requestModuleSolo(SoloCommands command)
{
    sqmix::processSoloRequestForModule<Comp>(this, command);
}

#ifdef __V1x
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
        int channel);
    void appendContextMenu(Menu *menu) override;
private:
    Mix4Module* mixModule;
};

void Mix4Widget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

    ManualMenuItem* manual = new ManualMenuItem(
        "ExFor manual",
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/exfor.md");
    menu->addChild(manual);
    
    MenuLabel *spacerLabel2 = new MenuLabel();
    menu->addChild(spacerLabel2);
    SqMenuItem_BooleanParam2 * item = new SqMenuItem_BooleanParam2(mixModule, Comp::PRE_FADERa_PARAM);
    item->text = "Send 1 Pre-Fader";
    menu->addChild(item);

    item = new SqMenuItem_BooleanParam2(mixModule, Comp::PRE_FADERb_PARAM);
    item->text = "Send 2 Pre-Fader";
    menu->addChild(item);

    item = new SqMenuItem_BooleanParam2(mixModule, Comp::CV_MUTE_TOGGLE);
    item->text = "Mute CV toggles on/off";
    menu->addChild(item);
}

static const float channelX = 21;
static const float dX = 36;
static const float labelX = 0; 
static const float channelY = 350;
static const float channelDy = 30;   
static float volY = 0;

void Mix4Widget::makeStrip(
    Mix4Module*,
    std::shared_ptr<IComposite> icomp,
    int channel)
{
    const float x = channelX + channel * dX;

    float y = channelY;

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::CHANNEL0_OUTPUT));

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::AUDIO0_INPUT));

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::MUTE0_INPUT));

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

    y -= (channelDy -1);
#if 0
    auto mute = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x-12, y-12),
        module,
        channel + Comp::MUTE0_PARAM);
    mute->addSvg("res/square-button-01.svg");
    mute->addSvg("res/square-button-02.svg");
    addParam(mute);
    muteY = y-12;

#else 
   
    const float mutx = x-11;
    const float muty = y-12;
    auto _mute = SqHelper::createParam<LEDBezel>(
        icomp,
        Vec(mutx, muty),
        module,
        channel + Comp::MUTE0_PARAM);
    addParam(_mute);

    addChild(createLight<MuteLight<SquinkyLight>>(
        Vec(mutx + 2.2, muty + 2),
        module,
        channel + Comp::MUTE0_LIGHT));
#endif
    
    y -= (channelDy-1);
    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(x-11, y-12),
        module,
        channel + Comp::SOLO0_LIGHT));
    std::string sLed = asset::system("res/ComponentLibrary/LEDBezel.svg");
    tog->addSvg(sLed.c_str(), true);
    tog->addSvg("res/SquinkyBezel.svg");
    tog->setHandler( [this, channel](bool ctrlKey) {
        sqmix::handleSoloClickFromUI<Comp>(mixModule, channel);
#if 0
         //printf("clicked on channel %d\n", channel);
        auto soloCommand =  SoloCommands(channel);
        if (ctrlKey) {
            soloCommand = SoloCommands(channel + int(SoloCommands::SOLO_0_MULTI));
        }
        printf("ui is requesting %f from click handler\n", (int) soloCommand);

        mixModule->requestSoloFromUI(soloCommand);
#endif
    });
    addChild(tog);
   
    const float extraDy = 5;
    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::GAIN0_PARAM));
    volY = y;

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::PAN0_PARAM));



    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::SENDb0_PARAM));

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::SEND0_PARAM));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1x
Mix4Widget::Mix4Widget(Mix4Module *module)
{
    setModule(module);
#else
Mix4Widget::Mix4Widget(Mix4Module *module) : ModuleWidget(module)
{
#endif
    mixModule = module;
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/mix4_panel.svg");
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    for (int i=0; i< Comp::numChannels; ++i) {
        makeStrip(module, icomp, i);
    }

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1x
Model *modelMix4Module = createModel<Mix4Module, Mix4Widget>("squinkylabs-mix4");
#else
Model *modelMix4Module = Model::create<Mix4Module,
    Mix4Widget>("Squinky Labs",
    "squinkylabs-mix4",
    "-- Mix4 --", RANDOM_TAG);
#endif
#endif

