
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _MIX8
#include "DrawTimer.h"
#include "Mix8.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

#include "ctrl/SqWidgets.h"

#include "ctrl/ToggleButtonV1.h"
#include "ctrl/ToggleManager2.h"

#ifdef _TIME_DRAWING
static DrawTimer drawTimer("Mix8");
#endif

using Comp = Mix8<WidgetComposite>;
using Manager = ToggleManager2<SqSvgParamToggleButton>;
/**
 */
struct Mix8Module : Module
{
public:
    Mix8Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> Mix8;
private:

};

void Mix8Module::onSampleRateChange()
{
}

Mix8Module::Mix8Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    Mix8 = std::make_shared<Comp>(this);
    onSampleRateChange();
    Mix8->init();
}

void Mix8Module::step()
{
    Mix8->step();
}

////////////////////
// module widget
////////////////////

struct Mix8Widget : ModuleWidget
{
    Mix8Widget(Mix8Module *);
    DECLARE_MANUAL("Mixer-8 manual", "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/mix8.md");

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
        Mix8Module*,
        std::shared_ptr<IComposite>,
        int channel,
        std::shared_ptr<Manager>);

    void makeMaster(Mix8Module* , std::shared_ptr<IComposite>);

    std::shared_ptr<Svg> buttonUp =  SqHelper::loadSvg("res/square-button-01.svg");
    std::shared_ptr<Svg> buttonDn =  SqHelper::loadSvg("res/square-button-02.svg");

#ifdef _TIME_DRAWING
    // Mix8: avg = 152.609857, stddev = 38.358624 (us) Quota frac=0.915659
    void draw(const DrawArgs &args) override
    {
        DrawLocker l(drawTimer);
        ModuleWidget::draw(args);
    }
#endif
};

static const float channelX = 43;
static const float dX = 34;
static const float labelX = 0; 
static const float channelY = 350;
static const float channelDy = 30; 
static float volY = 0;
static float muteY = 0;

void Mix8Widget::makeStrip(
    Mix8Module*,
    std::shared_ptr<IComposite> icomp,
    int channel,
    std::shared_ptr<Manager> mgr)
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
            "Out", 
            SqHelper::COLOR_WHITE);
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
    auto mute = SqHelper::createParam<SqSvgParamToggleButton>(
        icomp,
        Vec(x-12, y-12),
        module,
        channel + Comp::MUTE0_PARAM);
    mute->addFrame(buttonUp);
    mute->addFrame(buttonDn);
    addParam(mute);
    muteY = y-12;
   
    if (channel == 0) {
        addLabel(
            Vec(labelX+4, y-10),
            "M");
    }    

    y -= channelDy;
    auto solo = SqHelper::createParam<SqSvgParamToggleButton>(
        icomp,
        Vec(x-12, y-12),
        module,
        channel + Comp::SOLO0_PARAM);
    solo->addFrame(buttonUp);
    solo->addFrame(buttonDn);
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

void Mix8Widget::makeMaster(Mix8Module* module, std::shared_ptr<IComposite> icomp)
{
    float x = 0;
    float y = channelY;
    const float xL = 368;
    const float labelDy = -10;

    for (int channel = 0; channel<2; ++channel) {
        y = channelY;
        x = 312 + 15 + channel * dX;
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_EXPAND_INPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "X");
        }

        y -= channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_OUTPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "O",
             SqHelper::COLOR_WHITE);
        }

        y -= 2 * channelDy;
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_SEND_OUTPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "S",
             SqHelper::COLOR_WHITE);
        }

        y -= channelDy;
        addInput(createInputCentered<PJ301MPort>(
            Vec(x, y),
            module,
            channel + Comp::LEFT_RETURN_INPUT));
        if (channel == 0) {
            addLabel(Vec(xL, y+labelDy),
            "R");
        }
    }

    x = 312 + 15 + 15;
  
    auto mute = SqHelper::createParam<SqSvgParamToggleButton>(
        icomp,
        Vec(x-12, muteY),
        module,
        Comp::MASTER_MUTE_PARAM);
    mute->addFrame(buttonUp);
    mute->addFrame(buttonDn);
    addParam(mute);
  //  const float zz = SqHelper::getValue(mute);
  //  const float zz2 = mute->getValue();

    y = volY;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::MASTER_VOLUME_PARAM));
    addLabel(Vec(xL, y+labelDy),
            "M");
    
    y -= 55;
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x, y),
        module,
        Comp::RETURN_GAIN_PARAM)); 
    addLabel(Vec(xL, y+labelDy),
            "R"); 
    
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
Mix8Widget::Mix8Widget(Mix8Module *module)
{
    setModule(module);
    box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/mix8_panel.svg");
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    std::shared_ptr<Manager> mgr = std::make_shared<Manager>();
    for (int i=0; i<8; ++i) {
        makeStrip(module, icomp, i, mgr);
    }
    makeMaster(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelMix8Module = createModel<Mix8Module, Mix8Widget>("squinkylabs-mix8");
#endif

