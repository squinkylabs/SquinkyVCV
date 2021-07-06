
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _GMR
#include "GMR.h"
#include "ctrl/SqHelper.h"

#include "ctrl/SqMenuItem.h"
#include "ctrl/SqTooltips.h"
#include "ctrl/SqWidgets.h"


using Comp = GMR<WidgetComposite>;

/**
 */
struct GMRModule : Module
{
public:
    GMRModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> comp;
private:
};

void GMRModule::onSampleRateChange()
{
    float rate = SqHelper::engineGetSampleRate();
    comp->setSampleRate(rate);
}

GMRModule::GMRModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    comp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

  //  SqTooltips::changeParamQuantity<WaveformParamQuantity>(this, Comp::WAVEFORM_PARAM);

    onSampleRateChange();
    comp->init();
}

// TODO: proces
void GMRModule::step()
{
    comp->step();
}

////////////////////
// module widget
////////////////////

struct GMRWidget : ModuleWidget
{
    GMRWidget(GMRModule *);

    void addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
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
GMRWidget::GMRWidget(GMRModule * module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/blank_panel.svg");
#if 0
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(SqHelper::assetPlugin(pluginInstance, "res/blank_panel.svg")));
        addChild(panel);
    }
    #endif

    /*
      addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 0 * dy),
        module,
        Comp::PWM_INPUT));
        */

    addInput(createInput<PJ301MPort>(
        Vec(40, 200), 
        module,
        Comp::CLOCK_INPUT));
    addOutput(createOutput<PJ301MPort>(
        Vec(40, 300), 
        module,
        Comp::TRIGGER_OUTPUT));


    // screws
    #if 0
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    #endif
}

// Model *modelBasicModule = createModel<BasicModule, BasicWidget>("squinkylabs-basic");

Model *modelGMRModule = createModel<GMRModule, GMRWidget>("squinkylabs-gmr");
   

#endif

