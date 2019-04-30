
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

    const float x1 = 46;
    const float x2 = 210 - x1;

    const float y1 = 50;
    const float y2 = 120;
    const float y3 = 190;
    const float y4 = 246;
    const float yJacks = 300;
    const float labelY = 26;

    //const float xTest = 40;
   // const float yTest = 200;
   // const float dx = 40;

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y1),
        module,
        Comp::FC_PARAM));
    addLabel(
        Vec(x1-40, y1 + labelY),
        "Cutoff");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x2, y1),
        module,
        Comp::Q_PARAM));
     addLabel(
        Vec(x2-40, y1 + labelY),
        "Resonance");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y2),
        module,
        Comp::DRIVE_PARAM));
     addLabel(
        Vec(x1-40, y2 + labelY),
        "Drive");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x2, y2),
        module,
        Comp::STAGING_PARAM));
     addLabel(
        Vec(x2-40, y2 + labelY),
        "???");

     addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y3),
        module,
        Comp::VOICING_PARAM));
     addLabel(
        Vec(x1-40, y3 + labelY),
        "Voicing");

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(60, y4),
        module,
        Comp::TYPE_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getTypeNames());
    addParam(p);



    addInput(createInputCentered<PJ301MPort>(
        Vec(x1, yJacks),
        module,
        Comp::AUDIO_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x2, yJacks),
        module,
        Comp::AUDIO_OUTPUT));


    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
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

