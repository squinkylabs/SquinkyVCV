#include "ctrl/SqHelper.h"
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _DG
#include "daveguide.h"

using Comp = Daveguide<WidgetComposite>;

/**
 */
struct DGModule : Module
{
public:
    DGModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

   // Daveguide<WidgetComposite> dave;
    std::shared_ptr<Comp> comp;
private:
};

DGModule::DGModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    comp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    INFO("ChaosKittyModule::ChaosKittyModule( will call onSamplRateChange");
    onSampleRateChange();
  //  comp->init();
}

void DGModule::step()
{
    comp->step();
}

////////////////////
// module widget
////////////////////

struct DGWidget : ModuleWidget
{
    DGWidget(DGModule *);

    /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }


private:
   // DGModule* const module;
};




/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
DGWidget::DGWidget(DGModule *module)
{
    setModule(module);
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");
#if 0
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(pluginInstance, "res/blank_panel.svg")));
        addChild(panel);
    }
    #endif

    addLabel(Vec(35, 20), "Daveguide");

    addInput(createInputCentered<PJ301MPort>(
        Vec(40, 340),
        module,
        Daveguide<WidgetComposite>::AUDIO_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(120, 340),
        module,
        Daveguide<WidgetComposite>::AUDIO_OUTPUT));


    const float labelDeltaY = 25;
    const float gainX = 40;
    const float offsetX = 114;
    const float labelDeltaX = -20;
    const float y = 100;
    const float y2 = y + 70;

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(gainX, y),
        module, Daveguide<WidgetComposite>::OCTAVE_PARAM));
    addLabel(Vec(gainX + labelDeltaX, y + labelDeltaY), "octave");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(offsetX, y),
        module, Daveguide<WidgetComposite>::TUNE_PARAM));
    addLabel(Vec(offsetX + labelDeltaX, y + labelDeltaY), "tune");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(gainX, y2),
        module, Daveguide<WidgetComposite>::DECAY_PARAM));
    addLabel(Vec(gainX + labelDeltaX, y2 + labelDeltaY), "decay");

    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(offsetX, y2),
        module, Daveguide<WidgetComposite>::FC_PARAM));
    addLabel(Vec(offsetX + labelDeltaX, y2 + labelDeltaY), "filter");




    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  //  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


Model *modelDGModule = createModel<DGModule, DGWidget>("squinkylabs-dvg");

#endif

