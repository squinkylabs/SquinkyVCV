
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _WVCO
#include "WVCO.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/ToggleButton.h"


using Comp = WVCO<WidgetComposite>;

class DiscreteParamQuantity : public ParamQuantity {
public:
    DiscreteParamQuantity(const ParamQuantity& other, const std::vector<std::string>& str) : strings(str) {
        ParamQuantity* base = this;
        *base = other;
    }

    std::string getDisplayValueString() override {
        const unsigned int index = (unsigned int)(std::round(getValue()));
        if (index < strings.size()) {
            return strings[index];
        } else {
            assert(false);
            return "error";
        }
        
    }
private:
    std::vector<std::string> strings;
};

inline void subsituteDiscreteParamQuantity(const std::vector<std::string>& strings, Module& module, unsigned int paramNumber) {
    auto orig = module.paramQuantities[paramNumber];

     auto p = new DiscreteParamQuantity(*orig, strings);
    
    delete orig;
    module.paramQuantities[paramNumber] = p;
}

/**
 * This guy knows how to go out an patch in other instances
 */
class WvcoPatcher
{
public:
    static void go(ModuleWidget* hostWidget, Module* hostModule)
    {
        Module* otherModule = getLeftMatchingModule(hostModule); 
        ModuleWidget* otherModuleWidget = getWidgetForModule(otherModule);
        patchVOct(otherModuleWidget, hostWidget);
        patchModulator(otherModuleWidget, hostWidget);
    }

    static bool shouldShowMenu(Module* hostModule) 
    {
        return bool( getLeftMatchingModule(hostModule));
    }
private:

    static bool isPortPatched(PortWidget* portWidget) {
        auto cables = APP->scene->rack->getCablesOnPort(portWidget);
        return !cables.empty();
    }

    static void patchModulator(ModuleWidget* otherModuleWidget, ModuleWidget* myModuleWidget) {
        auto myFMPort = getInput(myModuleWidget, Comp::LINEAR_FM_INPUT);
        if (isPortPatched(myFMPort)) {
            WARN("my FM input already connected\n");
            return;
        }
        auto otherOutput = getOutput(otherModuleWidget, Comp::MAIN_OUTPUT);

        assert(myFMPort->type == PortWidget::INPUT);
        assert(otherOutput->type == PortWidget::OUTPUT);

         //   (output, input)
        patchBetweenPorts(otherOutput, myFMPort);
    }

    static void patchVOct(ModuleWidget* otherModuleWidget, ModuleWidget* myModuleWidget) {
        auto myVOctPort = getInput(myModuleWidget, Comp::VOCT_INPUT);

        if (!isPortPatched(myVOctPort)) {
            WARN("my voct not connected\n");
            return;
        }
        auto otherVOctPort = getInput(otherModuleWidget, Comp::VOCT_INPUT);

         if (isPortPatched(otherVOctPort)) {
            WARN("other V/Oct port already patched");
            return;
        }
        PortWidget* source = getOutputThatConnectsToThisInput(myVOctPort);
        patchBetweenPorts(source, otherVOctPort);
    }

    static Module* getLeftMatchingModule(Module* myModule) {
        Module* left = nullptr;
        auto leftExpander = myModule->leftExpander;
        if (leftExpander.module) {
            auto leftModule = leftExpander.module;
            if (leftModule->model == myModule->model) {
                // There is a copy of me to my left
                left = leftModule;
            }
        }
        return left;
    }

    static PortWidget* getOutputThatConnectsToThisInput(PortWidget* thisInput) {
        assert(thisInput->type == PortWidget::INPUT);
        auto cables = APP->scene->rack->getCablesOnPort(thisInput);
        assert(cables.size() == 1);
        auto cable = cables.begin();
        CableWidget* cw = *cable;
        PortWidget* ret =  cw->outputPort;
        assert(ret->type == PortWidget::OUTPUT);
        return ret;
    }

    static ModuleWidget* getWidgetForModule(Module* module) {
        auto rack = ::rack::appGet()->scene->rack;
        for (Widget* w2 : rack->moduleContainer->children) {
            ModuleWidget* modwid = dynamic_cast<ModuleWidget *>(w2);
            if (modwid) {
                if (modwid->module == module) {
                    return modwid;
                }
            }
        }
        return nullptr;
    }

    static void patchBetweenPorts(PortWidget* output, PortWidget* input) {
        if (isPortPatched(input)) {
            WARN("can't patch to input that is already patched");
            return;
        }
        CableWidget* cable = new CableWidget();
        // cable->color = color;
        cable->setOutput(output);
        cable->setInput(input);
        APP->scene->rack->addCable(cable);
    }

    static PortWidget* getInput(ModuleWidget* moduleWidget, int portId) {
        for (PortWidget* input : moduleWidget->inputs) {
            if (input->portId == portId) {
                return input;
            }
        }
        return nullptr;
    }

    static PortWidget* getOutput(ModuleWidget* moduleWidget, int portId) {
        for (PortWidget* output : moduleWidget->outputs) {
            if (output->portId == portId) {
                return output;
            }
        }
        return nullptr;
    }
};


/**
 */
struct WVCOModule : Module
{
public:
    WVCOModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> wvco;
private:

};

void WVCOModule::onSampleRateChange()
{
}

WVCOModule::WVCOModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    wvco = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    wvco->init();

    subsituteDiscreteParamQuantity(Comp::getWaveformNames(), *this, Comp::WAVE_SHAPE_PARAM);
}

void WVCOModule::step()
{
    wvco->step();
}

////////////////////
// module widget
////////////////////

struct WVCOWidget : ModuleWidget
{
    WVCOWidget(WVCOModule *);
    void appendContextMenu(Menu *menu) override;
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addKnobs(WVCOModule *module, std::shared_ptr<IComposite> icomp);
    void addJacks(WVCOModule *module, std::shared_ptr<IComposite> icomp);
    void addButtons(WVCOModule *module, std::shared_ptr<IComposite> icomp);

    WVCOModule* module = nullptr;
};

void WVCOWidget::appendContextMenu(Menu *menu)
{
    MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

    ManualMenuItem* manual = new ManualMenuItem(
        "Kitchen Sink manual",
        "https://github.com/squinkylabs/SquinkyVCV/blob/ck-1/docs/kitchen-sink.md");
    menu->addChild(manual);
    
    MenuLabel *spacerLabel2 = new MenuLabel();
    menu->addChild(spacerLabel2);
    SqMenuItem_BooleanParam2 * item = new SqMenuItem_BooleanParam2(module, Comp::SNAP_PARAM);
    item->text = "Envelope \"Snap\"";
    menu->addChild(item);
    item = new SqMenuItem_BooleanParam2(module, Comp::SNAP2_PARAM);
    item->text = "Extra Envelope \"Snap\"";
    menu->addChild(item);

    {
        if (WvcoPatcher::shouldShowMenu(module)) {
            auto item = new SqMenuItem( []() { return false; }, [this](){
                // float rawClockFalue = Comp::CLOCK_INPUT_PARAM
            //  float rawClockValue = ::rack::appGet()->engine->getParam(module, Comp::CLOCK_INPUT_PARAM);
            //  SeqClock::ClockRate rate =  SeqClock::ClockRate(int(std::round(rawClockValue)));
            //   const int div = SeqClock::clockRate2Div(rate);
                assert(module);
                WvcoPatcher::go(this, module);
            });

            item->text = "Hookup Modulator";
            menu->addChild(item);
        }
        
    }
}

const float knobLeftEdge = 24;
const float knobDeltaX = 46;
const float knobX1 = knobLeftEdge;
const float knobX2 = knobLeftEdge + 1 * knobDeltaX;
const float knobX3 = knobLeftEdge + 2 * knobDeltaX;
const float knobX4 = knobLeftEdge + 3 * knobDeltaX;

const float knobY1 = 60;
const float knobDeltaY = 70;
const float knobY2 = knobY1 + 1 *  knobDeltaY;
const float knobY3 = 14 + knobY1 + 2 *  knobDeltaY;
const float trimY = knobY3 + 66;
const float trimX = knobX2 - 4;

const float labelAboveKnob = 20;

void WVCOWidget::addKnobs(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    // first row
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX1, knobY1),
        module,
        Comp::OCTAVE_PARAM));
    addLabel(Vec(knobX1 - 13, knobY1 - labelAboveKnob), "Octave");

    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX2, knobY1),
        module,
        Comp::FREQUENCY_MULTIPLIER_PARAM));
    addLabel(Vec(knobX2 - 6, knobY1 - labelAboveKnob), "Ratio");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3, knobY1),
        module,
        Comp::FINE_TUNE_PARAM));
    addLabel(Vec(knobX3 - 4, knobY1 - labelAboveKnob), "Fine");

//
    addParam(SqHelper::createParam<Blue30SnapKnob>(
        icomp,
        Vec(knobX4, knobY1),
        module,
        Comp::WAVE_SHAPE_PARAM));
    addLabel(Vec(knobX4 - 8, knobY1 - labelAboveKnob), "Wave");

    // second row
    // 1 level
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX1, knobY2),
        module,
        Comp::OUTPUT_LEVEL_PARAM));
    addLabel(Vec(knobX1 - 8, knobY2 - labelAboveKnob), "Level");

    // 2 fm-0
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY2),
        module,
        Comp::LINEAR_FM_DEPTH_PARAM));
    addLabel(Vec(knobX2 - 10, knobY2 - labelAboveKnob), "Depth");
  
  // 3 fbck
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3, knobY2),
        module,
        Comp::FEEDBACK_PARAM));
    addLabel(Vec(knobX3 - 6, knobY2 - labelAboveKnob), "Fbck");

    // 4 SHAPE
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX4, knobY2),
        module,
        Comp::WAVESHAPE_GAIN_PARAM));
    addLabel(Vec(knobX4 - 10, knobY2 - labelAboveKnob), "Shape");


    // third row
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX1, knobY3),
        module,
        Comp::ATTACK_PARAM));
    addLabel(Vec(knobX1 + 4, knobY3 - labelAboveKnob), "A");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY3),
        module,
        Comp::DECAY_PARAM));
    addLabel(Vec(knobX2 + 4, knobY3 - labelAboveKnob), "D");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX3, knobY3),
        module,
        Comp::SUSTAIN_PARAM));
    addLabel(Vec(knobX3 + 4, knobY3 - labelAboveKnob), "S");

    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX4, knobY3),
        module,
        Comp::RELEASE_PARAM));
    addLabel(Vec(knobX4 + 4, knobY3 - labelAboveKnob), "R");

    // fourth row
    //PITCH MOD
    #if 1
    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(trimX, trimY),
        module,
        Comp::FM_DEPTH_PARAM));
    //addLabel(Vec(knobX4 , knobY4 - labelAboveKnob), "Mod");
#endif
}

class SqBlueButton : public ToggleButton 
{
public:
    SqBlueButton()
    {
        addSvg("res/square-button-01.svg");
        addSvg("res/square-button-02.svg");
    }
};

const float switchRow = knobY2 + 38;
const float buttonXShift = 3;

void WVCOWidget::addButtons(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    addParam(SqHelper::createParam<SqBlueButton>(
        icomp,
        Vec(knobX1 + buttonXShift, switchRow),
        module,
        Comp::ADSR_OUTPUT_LEVEL_PARAM));

    addParam(SqHelper::createParam<SqBlueButton>(
        icomp,
        Vec(knobX2 + buttonXShift, switchRow),
        module,
        Comp::ADSR_LFM_DEPTH_PARAM));
   
    addParam(SqHelper::createParam<SqBlueButton>(
        icomp,
        Vec(knobX3 + buttonXShift, switchRow),
        module,
        Comp::ADSR_FBCK_PARAM));

    addParam(SqHelper::createParam<SqBlueButton>(
        icomp,
        Vec(knobX4 + buttonXShift, switchRow),
        module,
        Comp::ADSR_SHAPE_PARAM));
}

const float jacksX1 = 24;
const float jacksDeltaX = 38;
const float jacksX2 = jacksX1 + 1 * jacksDeltaX;
const float jacksX3 = jacksX1 + 2 * jacksDeltaX;
const float jacksX4 = jacksX1 + 3 * jacksDeltaX;
const float jacksX5 = jacksX1 + 4 * jacksDeltaX;

const float jacksY1 = 276;
const float jacksY2 = jacksY1 + 46;



void WVCOWidget::addJacks(WVCOModule *module, std::shared_ptr<IComposite> icomp) {

    //-------------------------------- first row ----------------------------------
    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY1),
        module,
        Comp::GATE_INPUT));
    addLabel(Vec(jacksX1 - 8, jacksY1 - labelAboveKnob), "Gate");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX3, jacksY1),
        module,
        Comp::LINEAR_FM_DEPTH_INPUT));
    addLabel(Vec(jacksX3 - 12, jacksY1 - labelAboveKnob), "Depth");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX4, jacksY1),
        module,
        Comp::FEEDBACK_INPUT));
    addLabel(Vec(jacksX4 - 8, jacksY1 - labelAboveKnob), "Fbck");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX5, jacksY1),
        module,
        Comp::SHAPE_INPUT));
    addLabel(Vec(jacksX5 - 12, jacksY1 - labelAboveKnob), "Shape");

    //----------------------------- second row -----------------------
    addInput(createInput<PJ301MPort>(
        Vec(jacksX1, jacksY2),
        module,
        Comp::VOCT_INPUT));
    addLabel(Vec(jacksX1 - 11, jacksY2 - labelAboveKnob), "V/Oct");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX2, jacksY2),
        module,
        Comp::FM_INPUT));
    addLabel(Vec(jacksX2 - 3, jacksY2 - labelAboveKnob), "FM");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX3, jacksY2),
        module,
        Comp::LINEAR_FM_INPUT));
    addLabel(Vec(jacksX3 - 6, jacksY2 - labelAboveKnob), "LFM");

    addInput(createInput<PJ301MPort>(
        Vec(jacksX4, jacksY2),
        module,
        Comp::SYNC_INPUT));
    addLabel(Vec(jacksX4 - 9, jacksY2 - labelAboveKnob), "Sync");

    addOutput(createOutput<SqOutputJack>(
        Vec(jacksX5, jacksY2),
        module,
        Comp::MAIN_OUTPUT));
    addLabel(Vec(jacksX5 - 7, jacksY2 - labelAboveKnob), "Out");
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

WVCOWidget::WVCOWidget(WVCOModule *mod) : module(mod)
{
    setModule(module);
  //  box.size = Vec(14 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/wvco_panel.svg");

    addLabel(Vec(60, 14), "Kitchen Sink");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addKnobs(module, icomp);
    addButtons(module, icomp);
    addJacks(module, icomp);

}

Model *modelWVCOModule = createModel<WVCOModule, WVCOWidget>("squinkylabs-wvco");
#endif

