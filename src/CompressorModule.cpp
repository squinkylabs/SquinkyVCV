
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "Compressor.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"

using Comp = Compressor<WidgetComposite>;

/**
 */
struct CompressorModule : Module
{
public:
    CompressorModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> lim;
private:

};

CompressorModule::CompressorModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    lim = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    lim->init();
}

void CompressorModule::process(const ProcessArgs& args)
{
    lim->process(args);
}

void CompressorModule::onSampleRateChange()
{
    lim->onSampleRateChange();
}

////////////////////
// module widget
////////////////////

struct CompressorWidget : ModuleWidget
{
    CompressorWidget(CompressorModule *);
    DECLARE_MANUAL("Lim Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/compressor.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(CompressorModule *module, std::shared_ptr<IComposite> icomp);
    void addControls(CompressorModule *module, std::shared_ptr<IComposite> icomp);
};

void CompressorWidget::addControls(CompressorModule *module, std::shared_ptr<IComposite> icomp)
{
    const float knobX = 12;
    const float knobX2 = 45;
    const float knobY = 46;
    const float labelY = knobY - 20;
    const float dy = 50;

    addLabel(
        Vec(knobX, labelY + 0 * dy),
        "Atck");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 0 * dy),
        module,  Comp::ATTACK_PARAM));

    addLabel(
        Vec(knobX2, labelY + 0 * dy),
        "Rel");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY + 0 * dy),
        module,  Comp::RELEASE_PARAM));

    addLabel(
        Vec(knobX - 6, labelY + 1 * dy),
        "Thresh");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 1 * dy),
        module,  Comp::THRESHOLD_PARAM));

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX,  knobY + 2 * dy),
        module,
        Comp::RATIO_PARAM);
    p->box.size.x = 54;  // width
    p->box.size.y = 22;   
    p->text = "4:1";
    p->setLabels( {"4:1", "BP", "HP", "N"});
    addParam(p);

    addLabel(
        Vec(knobX, labelY + 3 * dy),
        "Dist Reduce");
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(knobX + 10, 5 + knobY + 3 * dy),
        module,  Comp::SECRET_PARAM));
}

void CompressorWidget::addJacks(CompressorModule *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float labelX = jackX - 6;
    const float jackY = 249;
    const float labelY = jackY - 17;
    const float dy = 40;

    addLabel(
        Vec(labelX+6, labelY + 0 * dy),
        "In");
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 0 * dy),
        module,
        Comp::AUDIO_INPUT));

    addLabel(
        Vec(labelX, labelY + 1 * dy),
        "Out");
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 1 * dy),
        module,
        Comp::AUDIO_OUTPUT));
#if 0
     addOutput(createOutput<PJ301MPort>(
        Vec(jackX, 40),
        module,
        Comp::DEBUG_OUTPUT));
 #endif
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

CompressorWidget::CompressorWidget(CompressorModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/compressor_panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addControls(module, icomp);
    addJacks(module, icomp);
    

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   // addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelCompressorModule = createModel<CompressorModule, CompressorWidget>("squinkylabs-comp");


