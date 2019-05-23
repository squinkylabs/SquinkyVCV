
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _FILT
#include "Filt.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/PopupMenuParamWidgetv1.h"

using Comp = Filt<WidgetComposite>;

/**
 */
struct FiltModule : Module
{
public:
    FiltModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> filt;
private:

};

void FiltModule::onSampleRateChange()
{
}


#ifdef __V1
FiltModule::FiltModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    filt = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 
#else
FiltModule::FiltModule()
    : Module(
        Comp::NUM_PARAMS,
        Comp::NUM_INPUTS,
        Comp::NUM_OUTPUTS,
        Comp::NUM_LIGHTS)
{
    filt = std::make_shared<Comp>(this);
#endif
    onSampleRateChange();
    filt->init();
}

void FiltModule::step()
{
    filt->step();
}

////////////////////
// module widget
////////////////////

struct FiltWidget : ModuleWidget
{
    FiltWidget(FiltModule *);
    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/sq5/docs/filter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addParams(FiltModule *module, std::shared_ptr<IComposite> icomp);
    void addTrimmers(FiltModule *module, std::shared_ptr<IComposite> icomp);
    void addJacks(FiltModule *module, std::shared_ptr<IComposite> icomp);
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
FiltWidget::FiltWidget(FiltModule *module)
{
    setModule(module);
#else
FiltWidget::FiltWidget(FiltModule *module) : ModuleWidget(module)
{
#endif
    box.size = Vec(14 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/filter_panel.svg");
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addParams(module, icomp);
    addTrimmers(module, icomp);
    addJacks(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void FiltWidget::addParams(FiltModule *module, std::shared_ptr<IComposite> icomp)
{
    const float deltaX = 46;        // was 50, 40 too close
    const float x1 = 32;
    const float x2 = x1 + deltaX;
    const float x3 = x2 + deltaX;
    const float x4 = x3 + deltaX;

    const float y1 = 80;
    const float y2 = 142;
    const float y3 = 186;
    const float yPole1 = y2 - 12;
    const float yVol1 = y1 - 12;
    const float dyPoles = 8;
    const float xLED = x4 + 26;
  
    const float labelDx = 22;
    const float labelY = -38;
   
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x1, y1),
        module,
        Comp::FC_PARAM));
    addLabel(
        Vec(x1+8-labelDx, y1 + labelY),
        "Fc");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x2, y1),
        module,
        Comp::Q_PARAM));
     addLabel(
        Vec(x2+10-labelDx, y1 + labelY),
        "Q");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x3, y1),
        module,
        Comp::DRIVE_PARAM));
     addLabel(
        Vec(x3-labelDx, y1 + labelY),
        "Drive");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x4, y1),
        module,
        Comp::MASTER_VOLUME_PARAM));
    addLabel(
        Vec(x4-labelDx, y1 + labelY),
        "Vol");

    for (int i=0; i<4; ++i) {
        addChild(createLightCentered<SmallLight<GreenLight>>(
            Vec(xLED, yVol1 + dyPoles * i ),
            module,
            Comp::VOL0_LIGHT + i));
    }

// second row
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x1, y2),
        module,
        Comp::STAGING_PARAM));
    addLabel(
        Vec(x1+2-labelDx, y2 + labelY),
        "Edge");


    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x2, y2),
        module,
        Comp::SPREAD_PARAM));
    addLabel(
        Vec(x2-labelDx, y2 + labelY),
        "Caps");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x3, y2),
        module,
        Comp::BASS_MAKEUP_PARAM));
    addLabel(
        Vec(x3+2-labelDx, y2 + labelY),
        "Bass");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(x4, y2),
        module,
        Comp::SLOPE_PARAM));
    addLabel(
        Vec(x4-labelDx, y2 + labelY),
        "Slope");

    for (int i=0; i<4; ++i) {
        addChild(createLightCentered<SmallLight<GreenLight>>(
            Vec(xLED, yPole1 + dyPoles * i ),
            module,
            Comp::SLOPE0_LIGHT + i));
    }
    
    // Third row
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(20, y3),
        module,
        Comp::TYPE_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getTypeNames());
    addParam(p);
 
    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(110, y3),
        module,
        Comp::VOICING_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getVoicingNames());
    addParam(p);
 } 

static const float jacksX1 = 30;
const float deltaXJack = 38;

void FiltWidget::addTrimmers(FiltModule *module, std::shared_ptr<IComposite> icomp)
 {
    const float yTrim = 240;
    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(jacksX1 + 0 * deltaXJack, yTrim),
        module,
        Comp::FC1_TRIM_PARAM));

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(jacksX1 + 1 * deltaXJack, yTrim),
        module,
        Comp::FC2_TRIM_PARAM));

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(jacksX1 + 2 * deltaXJack, yTrim),
        module,
        Comp::Q_TRIM_PARAM));

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(jacksX1 + 3 * deltaXJack, yTrim),
        module,
        Comp::DRIVE_TRIM_PARAM));
    
    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(jacksX1 + 4 * deltaXJack, yTrim),
        module,
        Comp::SLOPE_TRIM_PARAM));
 }

void FiltWidget::addJacks(FiltModule *module, std::shared_ptr<IComposite> icomp)
 {
    const float x1 = jacksX1;
    const float yJacks1 = 286;
    const float yJacks2 = 330;
   
    const float JackLabelY = -30;

    // row 1
    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + 0 * deltaXJack, yJacks1),
        module,
        Comp::CV_INPUT1));
    addLabel(
        Vec(x1 + 0 * deltaXJack -18, yJacks1 + JackLabelY),
        "CV1");

    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + 1 * deltaXJack, yJacks1),
        module,
        Comp::CV_INPUT2));
    addLabel(
        Vec(x1 + 1 * deltaXJack -18, yJacks1 + JackLabelY),
        "CV2");

    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + 2 * deltaXJack, yJacks1),
        module,
        Comp::Q_INPUT));
    addLabel(
        Vec(x1 + 2 * deltaXJack -14, yJacks1 + JackLabelY),
        "Q");
    
    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + 3 * deltaXJack, yJacks1),
        module,
        Comp::DRIVE_INPUT));
    addLabel(
        Vec(x1 + 3 * deltaXJack - 22, yJacks1 + JackLabelY),
        "drive");

    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + 4 * deltaXJack, yJacks1),
        module,
        Comp::SLOPE_INPUT));
    addLabel(
        Vec(x1 + 4 * deltaXJack - 22, yJacks1 + JackLabelY),
        "slope");

    // row2
    addInput(createInputCentered<PJ301MPort>(
        Vec(x1, yJacks2),
        module,
        Comp::L_AUDIO_INPUT));
    addLabel(
        Vec(x1-18, yJacks2 + JackLabelY),
        "In L");

    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + 1 * deltaXJack, yJacks2),
        module,
        Comp::R_AUDIO_INPUT));
    addLabel(
        Vec(x1 + 1 * deltaXJack -18, yJacks2 + JackLabelY),
        "In R");
  
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x1 + 2 * deltaXJack, yJacks2),
        module,
        Comp::L_AUDIO_OUTPUT));
    addLabel(
        Vec(x1 -2 + 2 * deltaXJack -18, yJacks2 + JackLabelY),
        "Out L");
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x1 + 3 * deltaXJack, yJacks2),
        module,
        Comp::R_AUDIO_OUTPUT));
    addLabel(
        Vec(x1 - 4 + 3 * deltaXJack -18, yJacks2 + JackLabelY),
        "Out R");
}


#ifdef __V1
Model *modelFiltModule = createModel<FiltModule, FiltWidget>("squinkylabs-filt");
#else

Model *modelFiltModule = Model::create<FiltModule,
    FiltWidget>("Squinky Labs",
    "squinkylabs-filt",
    "Stairway: Ladder Filter", FILTER_TAG, DISTORTION_TAG, DUAL_TAG);
#endif
#endif

