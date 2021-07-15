
#include <sstream>

#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _GMR
#include "GMR2.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqTooltips.h"
#include "ctrl/SqWidgets.h"
#include "grammar/GMRScreenHolder.h"
#include "grammar/GrammarRulePanel.h"
// #include "grammar/FakeScreen.h"

using Comp = GMR2<WidgetComposite>;

/**
 */
struct GMRModule : Module {
public:
    GMRModule();
    /**
     *
     * Overrides of Module functions
     */
    //   void step() override;
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> comp;
    //   StochasticGrammarPtr grammar;
private:
};

void GMRModule::onSampleRateChange() {
    float rate = SqHelper::engineGetSampleRate();
    comp->setSampleRate(rate);
}

GMRModule::GMRModule() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    comp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
    //  grammar = StochasticGrammar::getDemoGrammar();

    onSampleRateChange();
    comp->init();
}

void GMRModule::process(const ProcessArgs& args) {
    comp->process(args);
}
#if 0
void GMRModule::step()
{
    comp->step();
}
#endif

////////////////////
// module widget
////////////////////

struct GMRWidget : ModuleWidget {
    GMRWidget(GMRModule*);

    void addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
    }
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
GMRWidget::GMRWidget(GMRModule* module) {
    setModule(module);
    SqHelper::setPanel(this, "res/gmr_panel.svg");

    //   SQINFO("init width = %f", this->box.size.x);
    // make the active area almost full size, but don't cover up the jacks
#if 1
    const Vec gmrPos = Vec(0, 0);
    const Vec gmrSize = Vec(this->box.size.x, 335);
    Widget* p = new GMRScreenHolder(gmrPos, gmrSize);
    addChild(p);
#endif
    SQINFO("adding the jacks at the bottom my h=%f", this->box.size.y);
    ;
    addInput(createInput<PJ301MPort>(
        Vec(40, 340),
        module,
        Comp::CLOCK_INPUT));
    addOutput(createOutput<PJ301MPort>(
        Vec(90, 340),
        module,
        Comp::TRIGGER_OUTPUT));
}

Model* modelGMRModule = createModel<GMRModule, GMRWidget>("squinkylabs-gmr");

#endif
