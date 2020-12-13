
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SAMP
#include "Samp.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Samp<WidgetComposite>;

/**
 */
struct SampModule : Module
{
public:
    SampModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> samp;
private:

};

void SampModule::onSampleRateChange()
{
}

SampModule::SampModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    samp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    samp->init();
}

void SampModule::process(const ProcessArgs& args)
{
    samp->process(args);
}

////////////////////
// module widget
////////////////////

struct SampWidget : ModuleWidget
{
    SampWidget(SampModule *);
    DECLARE_MANUAL("Samp Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void addJacks(SampModule *module, std::shared_ptr<IComposite> icomp);
};

void SampWidget::addJacks(SampModule *module, std::shared_ptr<IComposite> icomp)
{
    float jacksY = 340;
    float jacksX = 15;
    float jacksDx = 40;
    float labelY = jacksY - 25;

    addLabel(
        Vec(jacksX + 0 * jacksDx - 5, labelY),
        "Out"
    );
    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY),
        module,
        Comp::AUDIO_OUTPUT));

    addLabel(
        Vec(jacksX + 1 * jacksDx - 10, labelY),
        "V/Oct"
    );
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY),
        module,
        Comp::PITCH_INPUT));

    addLabel(
        Vec(jacksX + 2 * jacksDx - 10, labelY),
        "Gate"
    );
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 2 * jacksDx, jacksY),
        module,
        Comp::GATE_INPUT));
    
     addLabel(
        Vec(jacksX + 3 * jacksDx - 6, labelY),
        "Vel"
    );
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 3 * jacksDx, jacksY),
        module,
        Comp::VELOCITY_INPUT));
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SampWidget::SampWidget(SampModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    addLabel( Vec(100, 50), "Sssssss");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelSampModule = createModel<SampModule, SampWidget>("squinkylabs-samp");
#endif

