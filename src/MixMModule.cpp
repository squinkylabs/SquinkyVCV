
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIXM
#include "DrawTimer.h"
#include "MixerModule.h"
#include "MixM.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqToggleLED.h"

#include "ctrl/SqWidgets.h"

#ifdef _TIME_DRAWING
static DrawTimer drawTimer("Mixm");
#endif

using Comp = MixM<WidgetComposite>;
#define WIDE 1
//#define _LABELS

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
    int getNumGroups() const override { return Comp::numGroups; }
    int getMuteAllParam() const override { return Comp::ALL_CHANNELS_OFF_PARAM; }
    int getSolo0Param() const override { return Comp::SOLO0_PARAM; }
    bool amMaster() override { return true; }
protected:
    void setExternalInput(const float*) override;
    void setExternalOutput(float*) override;
private:
    std::shared_ptr<Comp> MixM;

};

void MixMModule::onSampleRateChange()
{
    // update the anti-pop filters
    MixM->onSampleRateChange();
}

void MixMModule::setExternalInput(const float* buf)
{
    MixM->setExpansionInputs(buf);
}

void MixMModule::setExternalOutput(float* buf)
{

    assert(buf == nullptr);          // expander doesn't have an output expand
}

MixMModule::MixMModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    allocateSharedSoloState();

    MixM = std::make_shared<Comp>(this);
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

#ifdef _LABELS
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    #endif

    void makeStrip(
        MixMModule*,
        std::shared_ptr<IComposite>,
        int channel);
    void makeMaster(MixMModule* , std::shared_ptr<IComposite>); 

    void appendContextMenu(Menu *menu) override;

#ifdef _TIME_DRAWING
    // Mixm: avg = 106.471380, stddev = 34.237331 (us) Quota frac=0.638828
    void draw(const DrawArgs &args) override
    {
        DrawLocker l(drawTimer);
        ModuleWidget::draw(args);
    }
#endif
private:
    MixMModule* mixModule;          
};

void MixMWidget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

    ManualMenuItem* manual = new ManualMenuItem(
        "Form manual",
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/form.md");
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

#ifdef _LABELS
static const float labelX = 0; 
#endif

static const float channelX = 42;
static const float dX = 34;
static const float channelY = 350;
static const float channelDy = 30; 
static float volY = 0;
const float extraDy = 5;
static float muteY = 0;

void MixMWidget::makeStrip(
    MixMModule* module,
    std::shared_ptr<IComposite> icomp,
    int channel)
{
    const float x = channelX + channel * dX;

    float y = channelY;

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::CHANNEL0_OUTPUT));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "Out");
    }
#endif

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::AUDIO0_INPUT));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX+6, y-10),
            "In");
    }
#endif
    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::MUTE0_INPUT));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX+2, y-10),
            "M");
    }
#endif

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::LEVEL0_INPUT));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX, y-10),
            "Vol");
    }
#endif

    y -= channelDy;
    addInput(createInputCentered<PJ301MPort>(
        Vec(x, y),
        module,
        channel + Comp::PAN0_INPUT));

#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX-2, y-10),
            "Pan");
    }
#endif

    y -= (channelDy -1);

    const float mutex = x-11;       // was 12
    const float mutey = y-12;
    addParam(SqHelper::createParam<LEDBezel>(
        icomp,
        Vec(mutex, mutey),
        module,
        channel + Comp::MUTE0_PARAM));
  

    addChild(createLight<MuteLight<SquinkyLight>>(
        Vec(mutex + 2.2, mutey + 2),
        module,
        channel + Comp::MUTE0_LIGHT));
    muteY = y-12;
#ifdef _LABELS   
    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "M");
    }    
#endif

    y -= (channelDy - 1);

    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(x-11, y-12),
        module,
        channel + Comp::SOLO0_LIGHT));

    std::string sLed = asset::system("res/ComponentLibrary/LEDBezel.svg");
    tog->addSvg(sLed.c_str(), true);
    tog->addSvg("res/SquinkyBezel.svg");

    tog->setHandler( [this, channel](bool ctrlKey) {
        sqmix::handleSoloClickFromUI<Comp>(mixModule, channel, ctrlKey);
    });
    addChild(tog);
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "S");
    }    
#endif
    
    y -= (channelDy + extraDy);

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::GAIN0_PARAM));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX-2, y-10),
            "Vol");
    }
#endif
    volY = y;

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::PAN0_PARAM));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "Pan");
    }
#endif

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::SENDb0_PARAM));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "AX2");
    }
#endif

    y -= (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        channel + Comp::SEND0_PARAM));
#ifdef _LABELS
    if (channel == 0) {
        addLabel(
            Vec(labelX-4, y-10),
            "AX1");
    }
#endif
}

void MixMWidget::makeMaster(MixMModule* module, std::shared_ptr<IComposite> icomp)
{
    float x = 0;
    float y = channelY;
    const float x0 = 160;
#ifdef _LABELS
    const float xL = 215  + (WIDE * 15);
    const float labelDy = -10;
#endif

    for (int channel = 0; channel<2; ++channel) {
        y = channelY;
        x = x0 + 13 + (channel * dX) + (WIDE * 15);

        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_OUTPUT));
#ifdef _LABELS
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "O"
            //,SqHelper::COLOR_WHITE
            );
        }
#endif

        y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_SENDb_OUTPUT));
#ifdef _LABELS
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "S2"
            //,SqHelper::COLOR_WHITE
            );
        }
#endif

        y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_SEND_OUTPUT));
#ifdef _LABELS
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "S1"
            //,SqHelper::COLOR_WHITE
            );
        }
#endif

        y -= channelDy;
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_RETURNb_INPUT));
#ifdef _LABELS
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "R2");
        }
#endif

        y -= channelDy;
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_RETURN_INPUT));
#ifdef _LABELS
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "R1");
        }
#endif
    }

    x = x0 + 15 + 16  + (WIDE * 15);

    // Big Mute button
    const float mutex = x-11;
    const float mutey = muteY;

    auto mute = SqHelper::createParam<LEDBezelLG>(
        icomp,
        Vec(mutex - 6, mutey - 21),
        module,
        Comp::MASTER_MUTE_PARAM);
    addParam(mute);

    auto light = (createLight<MuteLight<SquinkyLight>>(
        Vec(mutex + 3.2 - 6, mutey + 3 - 21),
        module, Comp::MUTE_MASTER_LIGHT));
    // 30 too big
    light->box.size.x = 26;
    light->box.size.y = 26;
    addChild(light);
    muteY = y-12;
    
    y = volY;
  
    addParam(SqHelper::createParamCentered<Rogan2PSBlue>(
        icomp,
        Vec(x, y-12),
        module,
        Comp::MASTER_VOLUME_PARAM));

  
     y -=  (channelDy + extraDy) * 2;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::RETURN_GAINb_PARAM));
#ifdef _LABELS
    addLabel(Vec(xL-3, y+labelDy),
            "R2");
#endif
    y -=  (channelDy + extraDy);
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::RETURN_GAIN_PARAM));
#ifdef _LABELS
    addLabel(Vec(xL-3, y+labelDy),
            "R1");
#endif
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
MixMWidget::MixMWidget(MixMModule *module)
{
    setModule(module);
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

