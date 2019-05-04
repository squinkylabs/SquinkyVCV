
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
    addJacks(module, icomp);

       // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

}



void FiltWidget::addParams(FiltModule *module, std::shared_ptr<IComposite> icomp)
{
    const float x1 = 46;
    const float x2 = 210 / 2;
    const float x3 = 210 - x1;

    const float y1 = 50;
    const float y2 = 120;
    const float y3 = 190;
    const float labelDx = 28;
    const float labelY = 26;
   

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y1),
        module,
        Comp::FC_PARAM));
    addLabel(
        Vec(x1-labelDx, y1 + labelY),
        "Cutoff");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x2, y1),
        module,
        Comp::Q_PARAM));
     addLabel(
        Vec(x2-labelDx, y1 + labelY),
        "Q / Reso");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x3, y1),
        module,
        Comp::DRIVE_PARAM));
     addLabel(
        Vec(x3-labelDx, y1 + labelY),
        "Drive");

// second row
    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y2),
        module,
        Comp::STAGING_PARAM));
    addLabel(
        Vec(x1-labelDx, y2 + labelY),
        "Edge");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x2, y2),
        module,
        Comp::SPREAD_PARAM));
    addLabel(
        Vec(x2-labelDx, y2 + labelY),
        "Caps");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x3, y2),
        module,
        Comp::POLES_PARAM));
    addLabel(
        Vec(x3-labelDx, y2 + labelY),
        "Poles");


// Third row
    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y3),
        module,
        Comp::BASS_MAKEUP_PARAM));
    addLabel(
        Vec(x1-labelDx, y3 + labelY),
        "Bass");

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(x2, y3),
        module,
        Comp::TYPE_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getTypeNames());
    addParam(p);
 
    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(x2, y3 + 40),
        module,
        Comp::VOICING_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getVoicingNames());
    addParam(p);
 } 

void FiltWidget::addJacks(FiltModule *module, std::shared_ptr<IComposite> icomp)
 {
    const float x1 = 46;
    const float yJacks = 330;
    const float deltaXJack = 30;
    const float JackLabelY = -30;

    addInput(createInputCentered<PJ301MPort>(
        Vec(x1, yJacks),
        module,
        Comp::L_AUDIO_INPUT));
    addLabel(
        Vec(x1-18, yJacks + JackLabelY),
        "In");

    addInput(createInputCentered<PJ301MPort>(
        Vec(x1 + deltaXJack, yJacks),
        module,
        Comp::CV_INPUT));
    addLabel(
        Vec(x1 + deltaXJack -18, yJacks + JackLabelY),
        "CV");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x1 + 2 * deltaXJack, yJacks),
        module,
        Comp::L_AUDIO_OUTPUT));
    addLabel(
        Vec(x1 + 2 * deltaXJack -18, yJacks + JackLabelY),
        "Out");

 }


#ifdef __V1
Model *modelFiltModule = createModel<FiltModule, FiltWidget>("squinkylabs-filt");
#else

Model *modelFiltModule = Model::create<FiltModule,
    FiltWidget>("Squinky Labs",
    "squinkylabs-filt",
    "-- filt --", FILTER_TAG);
#endif
#endif

