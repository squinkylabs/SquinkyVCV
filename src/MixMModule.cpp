
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIXM
#include "MixerModule.h"


#include "MixM.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/ToggleButton.h"

#include "ctrl/SqWidgets.h"

using Comp = MixM<WidgetComposite>;

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
     bool amMaster() override { return true; }

    std::shared_ptr<Comp> MixM;
protected:
    void setExternalInput(const float*) override;
    void setExternalOutput(float*) override;
private:

};

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


#ifdef __V1
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
        MixMModule*,
        std::shared_ptr<IComposite>,
        int channel,
        std::shared_ptr<ToggleManager>);
    void makeMaster(MixMModule* , std::shared_ptr<IComposite>);           
};

static const float channelX = 42;
static const float dX = 34;
static const float labelX = 0; 
static const float channelY = 350;
static const float channelDy = 30; 
static float volY = 0;
static float muteY = 0;

void MixMWidget::makeStrip(
    MixMModule*,
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
            "Lvl");
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
    auto mute = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x-12, y-12),
        module,
        channel + Comp::MUTE0_PARAM);
    mute->addSvg("res/square-button-01.svg");
    mute->addSvg("res/square-button-02.svg");
    addParam(mute);
    muteY = y-12;
   
    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "M");
    }    

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

    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "S");
    }    

    const float extraDy = 6;
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
            "Aux");
    }
}

void MixMWidget::makeMaster(MixMModule* module, std::shared_ptr<IComposite> icomp)
{
    float x = 0;
    float y = channelY;
    const float x0 = 160;

    for (int channel = 0; channel<2; ++channel) {
        y = channelY;
        x = x0 + 15 + channel * dX;
    #if 0
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_EXPAND_INPUT));
#endif

        y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_OUTPUT));
    }

    x = x0 + 15 + 15;

    auto mute = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(x-12, muteY),
        module,
        Comp::MASTER_MUTE_PARAM);
    mute->addSvg("res/square-button-01.svg");
    mute->addSvg("res/square-button-02.svg");
    addParam(mute);

     y -= channelDy;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, volY),
        module,
        Comp::MASTER_VOLUME_PARAM));
   
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
MixMWidget::MixMWidget(MixMModule *module)
{
    setModule(module);
#else
MixMWidget::MixMWidget(MixMModule *module) : ModuleWidget(module)
{
#endif
    box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/mixm_panel.svg");
     std::shared_ptr<IComposite> icomp = Comp::getDescription();

    std::shared_ptr<ToggleManager> mgr = std::make_shared<ToggleManager>();
    for (int i=0; i<Comp::numChannels; ++i) {
        makeStrip(module, icomp, i, mgr);
    }
    makeMaster(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1
Model *modelMixMModule = createModel<MixMModule, MixMWidget>("squinkylabs-MixM");
#else
Model *modelMixMModule = Model::create<MixMModule,
    MixMWidget>("Squinky Labs",
    "squinkylabs-MixM",
    "-- MixM --", RANDOM_TAG);
#endif
#endif

