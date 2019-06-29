
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIXM
#include "MixerModule.h"


#include "MixM.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/ToggleButton.h"
#include "ctrl/SqToggleLED.h"

#include "ctrl/SqWidgets.h"

using Comp = MixM<WidgetComposite>;
#define WIDE 1

/**
 */
struct MixMModule : MixerModule
{
public:
    MixMModule();
    /**
     *
     * Overrides of Module functions
     */
    void onSampleRateChange() override;

    // Override MixerModule
    void internalProcess() override;
    void requestModuleSolo( SoloCommands) override;
    bool amMaster() override { return true; }


    
protected:
    void setExternalInput(const float*) override;
    void setExternalOutput(float*) override;
private:
    std::shared_ptr<Comp> MixM;

};

void MixMModule::requestModuleSolo(SoloCommands command)
{
    sqmix::processSoloRequestForModule<Comp>(this, command);
}

void MixMModule::onSampleRateChange()
{
    // TODO: do we need this?
}

void MixMModule::setExternalInput(const float* buf)
{
   // printf("mixm, set external input\n"); fflush(stdout);
    MixM->setExpansionInputs(buf);
}

void MixMModule::setExternalOutput(float* buf)
{
    // printf("mixm, set external outpu\n"); fflush(stdout);
    assert(buf == nullptr);          // expander doesn't have an output expand
}


#ifdef __V1x
MixMModule::MixMModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
MixMModule::MixMModule()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS)
{
#endif
    MixM = std::make_shared<Comp>(this);
    onSampleRateChange();
    MixM->init();
}

void MixMModule::internalProcess()
{
    MixM->step();
}

////////////////////
// module widget
////////////////////

struct MixMWidget : ModuleWidget
{
    MixMWidget(MixMModule *);

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
        MixMModule*,
        std::shared_ptr<IComposite>,
        int channel);
    void makeMaster(MixMModule* , std::shared_ptr<IComposite>); 

    void appendContextMenu(Menu *menu) override;
private:
    MixMModule* mixModule;          
};

void MixMWidget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

    ManualMenuItem* manual = new ManualMenuItem("Mix-4M manual", "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/booty-shifter.md");
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

static const float channelX = 42;
static const float dX = 34;
static const float labelX = 0; 
static const float channelY = 350;
static const float channelDy = 30; 
static float volY = 0;
const float extraDy = 6;
static float muteY = 0;

// From VCV Mutes
template <typename BASE>
struct MuteLight : BASE {
	MuteLight() {
	  this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

void MixMWidget::makeStrip(
    MixMModule* module,
    std::shared_ptr<IComposite> icomp,
    int channel)
{
    const float x = channelX + channel * dX;

    float y = channelY;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::AUDIO0_INPUT));

    if (channel == 0) {
        addLabel(
            Vec(labelX+6, y-10),
            "In");
    }

    y -= channelDy;
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::CHANNEL0_OUTPUT));

    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "Out");
    }

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::MUTE0_INPUT));

    if (channel == 0) {
        addLabel(
            Vec(labelX+2, y-10),
            "Mt");
    }

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::LEVEL0_INPUT));

    if (channel == 0) {
        addLabel(
            Vec(labelX, y-10),
            "Vol");
    }

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::PAN0_INPUT));

    if (channel == 0) {
        addLabel(
            Vec(labelX-2, y-10),
            "Pan");
    }

    y -= channelDy;
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

    const float mutex = x-12;
    const float mutey = y-12;
    auto mute = SqHelper::createParam<LEDBezel>(
        icomp,
        Vec(mutex, mutey),
        module,
        channel + Comp::MUTE0_PARAM);
    addParam(mute);

    addChild(createLight<MuteLight<GreenLight>>(
        Vec(mutex + 2.2, mutey + 2),
        module,
        channel + Comp::MUTE0_LIGHT));
    muteY = y-12;
#endif
   
    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "M");
    }    

    y -= channelDy;
    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(x-12, y-12),
        module,
        channel + Comp::SOLO0_LIGHT));
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    tog->setHandler( [this, module, channel](bool ctrlKey) {
        sqmix::handleSoloClickFromUI<Comp>(mixModule, channel);
    });
    addChild(tog);

    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "S");
    }    

    
    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::GAIN0_PARAM));
    if (channel == 0) {
        addLabel(
            Vec(labelX-2, y-10),
            "Vol");
    }
    volY = y;

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::PAN0_PARAM));
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "Pan");
    }

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::SEND0_PARAM));
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "AX1");
    }

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::SENDb0_PARAM));
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "AX2");
    }
}

void MixMWidget::makeMaster(MixMModule* module, std::shared_ptr<IComposite> icomp)
{
    float x = 0;
    float y = channelY;
    const float x0 = 160;
    const float xL = 215  + (WIDE * 15);
    const float labelDy = -10;

    for (int channel = 0; channel<2; ++channel) {
        y = channelY;
        x = x0 + 15 + (channel * dX) + (WIDE * 15);

       // y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_OUTPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "O"
            //,SqHelper::COLOR_WHITE
            );
        }

        y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_SEND_OUTPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "S1"
            //,SqHelper::COLOR_WHITE
            );
        }

        y -= channelDy;
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_RETURN_INPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "R1");
        }

        y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_SENDb_OUTPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "S2"
            //,SqHelper::COLOR_WHITE
            );
        }

        y -= channelDy;
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_RETURNb_INPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "R2");
        }
    }

    x = x0 + 15 + 17  + (WIDE * 15);

#if 0
    auto mute = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x-12, muteY),
        module,
        Comp::MASTER_MUTE_PARAM);
    mute->addSvg("res/square-button-01.svg");
    mute->addSvg("res/square-button-02.svg");
    addParam(mute);
#else
    const float mutex = x-12;
    const float mutey = muteY;
    auto mute = SqHelper::createParam<LEDBezel>(
        icomp,
        Vec(mutex, mutey),
        module,
        Comp::MASTER_MUTE_PARAM);
    addParam(mute);

    addChild(createLight<MuteLight<GreenLight>>(
        Vec(mutex + 2.2, mutey + 2),
        module, Comp::MUTE_MASTER_LIGHT));
    muteY = y-12;
#endif




    y = volY;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::MASTER_VOLUME_PARAM));

    y -=  (channelDy + extraDy) * 2;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::RETURN_GAIN_PARAM));
     y -=  (channelDy + extraDy);;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::RETURN_GAINb_PARAM));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1x
MixMWidget::MixMWidget(MixMModule *module)
{
    setModule(module);
#else
MixMWidget::MixMWidget(MixMModule *module) : ModuleWidget(module)
{
#endif
    mixModule = module;
    box.size = Vec((16 + WIDE) * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/mixm_panel.svg");
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    for (int i=0; i<Comp::numChannels; ++i) {
        makeStrip(module, icomp, i);
    }
    makeMaster(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1x
Model *modelMixMModule = createModel<MixMModule, MixMWidget>("squinkylabs-mixm");
#else
Model *modelMixMModule = Model::create<MixMModule,
    MixMWidget>("Squinky Labs",
    "squinkylabs-mixm",
    "-- MixM --", RANDOM_TAG);
#endif
#endif

