
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _F2
#include "F2.h"
#include "F2_Poly.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqTooltips.h"
#include "SqStream.h"


using Comp = F2_Poly<WidgetComposite>;


class OnOffQuantity :  public SqTooltips::SQParamQuantity {
public:
    OnOffQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
    }
    std::string getDisplayValueString() override {
        auto value = getValue();
        return value < .5 ? "Off" : "On";
    }
};



class TopologyQuantity :  public SqTooltips::SQParamQuantity {
public:
    TopologyQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
    }
    std::string getDisplayValueString() override {
        int value = int( std::round(getValue()));
        std::string tip;
        switch (value) {
            case 0:
                tip = "12 dB/octave multi-mode";
                break;
            case 1:
                tip = "24 dB/octave multi-mode";
                break;
            case 2:
                tip = "two 12 dB/octave in parallel";
                break;
            case 3:
                tip = "two 12 dB/octave subtracted";
                break;
            default:
                assert(false);
        }
        return tip;
    }
};

class ModeQuantity :  public SqTooltips::SQParamQuantity {
public:
    ModeQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
    }
    std::string getDisplayValueString() override {
        int value = int( std::round(getValue()));
        std::string tip;
        switch (value) {
            case 0:
                tip = "lowpass";
                break;
            case 2:
                tip = "highpass";
                break;
            case 1:
                tip = "bandpass";
                break;
            case 3:
                tip = "notch";
                break;
            default:
                assert(false);
        }
        return tip;
    }
};

class AttenQuantity :  public SqTooltips::SQParamQuantity {
public:
    AttenQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
    }
    std::string getDisplayValueString() override {
        float value =getValue();
        SqStream str;
        str.precision(0);
        str.add(value * 100);
        str.add("%");
        return str.str();
    }
};

/**
 */
struct F2Module : Module
{
public:
    F2Module();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;
private:

};

void F2Module::onSampleRateChange()
{
}

F2Module::F2Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    blank->init();

    SqTooltips::changeParamQuantity<OnOffQuantity>(this, Comp::LIMITER_PARAM);
    SqTooltips::changeParamQuantity<TopologyQuantity>(this, Comp::TOPOLOGY_PARAM);
    SqTooltips::changeParamQuantity<ModeQuantity>(this, Comp::MODE_PARAM);
    SqTooltips::changeParamQuantity<AttenQuantity>(this, Comp::FC_TRIM_PARAM);
}

void F2Module::process(const ProcessArgs& args)
{
    blank->process(args);
}

////////////////////
// module widget
////////////////////

struct F2Widget : ModuleWidget
{
    F2Widget(F2Module *);
 //   DECLARE_MANUAL("Basic VCF Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/f2.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void appendContextMenu(Menu *menu) override;

    void addJacks(F2Module *module, std::shared_ptr<IComposite> icomp);
    void addKnobs(F2Module *module, std::shared_ptr<IComposite> icomp);
    
};

void F2Widget::appendContextMenu(Menu* theMenu) 
{
    MenuLabel *spacerLabel = new MenuLabel();
    theMenu->addChild(spacerLabel);
    ManualMenuItem* manual = new ManualMenuItem("Basic VCF Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/f2.md");
    theMenu->addChild(manual);
    
    SqMenuItem_BooleanParam2 * item = new SqMenuItem_BooleanParam2(module, Comp::CV_UPDATE_FREQ);
    item->text = "CV Fidelity";
    theMenu->addChild(item);
}

void F2Widget::addKnobs(F2Module *module, std::shared_ptr<IComposite> icomp)
{
    const float knobX = 12;
    const float knobX2 = 45;
    const float knobY = 40;
    const float labelY = knobY - 20;
    const float dy = 50;

    addLabel(
        Vec(knobX, labelY + 0 * dy),
        "Fc");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 0 * dy),
        module,  Comp::FC_PARAM));

    addLabel(
        Vec(knobX2+3, labelY + 0 * dy),
        "Q");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY + 0 * dy),
        module,  Comp::Q_PARAM));
    
    addLabel(
        Vec(knobX+3, labelY + 1 * dy),
        "R");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 1 * dy),
        module,  Comp::R_PARAM));

    addLabel(
        Vec(knobX2 - 5, labelY + 1 * dy),
        "Limit");
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(knobX2 + 10, 5 + knobY + 1 * dy),
        module,  Comp::LIMITER_PARAM));

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX,  knobY + 2 * dy),
        module,
        Comp::MODE_PARAM);
    p->box.size.x = 66;  // width
    p->box.size.y = 22;   
    p->text = "LP";
    p->setLabels( {"LP", "BP", "HP", "N"});
    addParam(p);

    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX,  knobY + 3 * dy),
        module,
        Comp::TOPOLOGY_PARAM);
    p->box.size.x = 66;  // width was 54
    p->box.size.y = 22;   
    p->text = "12dB";
    p->setLabels( {"12dB", "24dB", "Par", "Par -"});
    addParam(p);

    // just for test
#if 0
    addChild(createLight<MediumLight<GreenLight>>(
            Vec(40, 18),
            module,
            Comp::LIGHT_TEST));
#endif
}

void F2Widget::addJacks(F2Module *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float jackX2 = 48;

    const float jackY = 200;
    const float labelY = jackY - 18;
    const float dy = 45;

     addLabel(
        Vec(jackX-3 , labelY + 1 * dy),
        "Fc");
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 1 * dy),
        module,
        Comp::FC_INPUT));

    addParam(SqHelper::createParam<SqTrimpot24>(
        icomp,
        Vec(jackX2, jackY + 1 * dy),
        module,
        Comp::FC_TRIM_PARAM));

    addLabel(
        Vec(jackX , labelY + 2 * dy),
        "Q");
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy),
        module,
        Comp::Q_INPUT));

    addLabel(
        Vec(jackX2 , labelY + 2 * dy),
        "R");
    addInput(createInput<PJ301MPort>(
        Vec(jackX2, jackY + 2 * dy),
        module,
        Comp::R_INPUT));
#if 0   // normal
    addLabel(
        Vec(jackX , labelY + 3 * dy),
        "In");
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 3 * dy),
        module,
        Comp::AUDIO_INPUT));

    addLabel(
        Vec(jackX2 - 5 , labelY + 3 * dy),
        "Out");
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX2, jackY + 3 * dy - .5),
        module,
        Comp::AUDIO_OUTPUT));
#else       // move the jacks up for easier viewing
  addLabel(
        Vec(jackX , labelY + 2.5 * dy),
        "In");
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 2.5 * dy),
        module,
        Comp::AUDIO_INPUT));

    addLabel(
        Vec(jackX2 - 5 , labelY + 3.5 * dy),
        "Out");
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX2, jackY + 2.5 * dy - .5),
        module,
        Comp::AUDIO_OUTPUT));

#endif

};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

F2Widget::F2Widget(F2Module *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/f2-panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);
    addKnobs(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelF2Module = createModel<F2Module, F2Widget>("squinkylabs-f2");
#endif

