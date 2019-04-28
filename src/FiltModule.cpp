
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _FILT
#include "Filt.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

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
    const float y1 = 100;
    const float y2 = 300;

    const float xTest = 40;
    const float yTest = 200;
    const float dx = 40;

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x1, y1),
        module,
        Comp::FC_PARAM));
    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(x2, y1),
        module,
        Comp::Q_PARAM));


    addInput(createInputCentered<PJ301MPort>(
        Vec(x1, y2),
        module,
        Comp::AUDIO_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(x2, y2),
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

