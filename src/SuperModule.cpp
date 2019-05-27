#include "Squinky.hpp"

#ifdef _SUPER
#include "WidgetComposite.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqMenuItem.h"
#include "Super.h"
#include "ctrl/ToggleButton.h"
#include "ctrl/SemitoneDisplay.h"
#include "IMWidgets.hpp"

#include <sstream>

using Comp = Super<WidgetComposite>;

/**
 */
struct SuperModule : Module
{
public:
    SuperModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> super;
};

void SuperModule::onSampleRateChange()
{
}

#ifdef __V1
SuperModule::SuperModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    super = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);

#else
SuperModule::SuperModule()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS),
    super(std::make_shared<Comp>(this))
{
#endif
    onSampleRateChange();
    super->init();
}

void SuperModule::step()
{
    super->step();
}

////////////////////
// module widget
////////////////////

struct superWidget : ModuleWidget
{
    superWidget(SuperModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void step() override
    {
        semitoneDisplay.step();
        ModuleWidget::step();
    }

    void addPitchKnobs(SuperModule *, std::shared_ptr<IComposite>);
    void addOtherKnobs(SuperModule *, std::shared_ptr<IComposite>);
    void addJacks(SuperModule *);
    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/saws.md");

    SemitoneDisplay semitoneDisplay;
};

const float col1 = 40;
const float col2 = 110;

const float row1 = 71;
const float row2 = 134;
const float row3 = 220;
const float row4 = 250;

const float jackRow1 = 290;
const float jackRow2 = 332;

const float labelOffsetBig = -40;
const float labelOffsetSmall = -32;

void superWidget::addPitchKnobs(SuperModule* module, std::shared_ptr<IComposite> icomp)
{
    // Octave
    Rogan1PSBlue* oct = SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(col1, row1),
        module, 
        Comp::OCTAVE_PARAM);
    oct->snap = true;
    oct->smooth = false;
    addParam(oct);
    Label* l = addLabel(
        Vec(col1 - 23, row1 + labelOffsetBig),
        "Oct");
    semitoneDisplay.setOctLabel(l, Super<WidgetComposite>::OCTAVE_PARAM);

    // Semi
    auto semi = SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(col2, row1),
        module,
        Super<WidgetComposite>::SEMI_PARAM);
    semi->snap = true;
    semi->smooth = false;
    addParam(semi);
    l = addLabel(
        Vec(col2 - 27, row1 + labelOffsetBig),
        "Semi");
    semitoneDisplay.setSemiLabel(l, Super<WidgetComposite>::SEMI_PARAM);

    // Fine
    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(col1, row2),
        module,
        Comp::FINE_PARAM));
    addLabel(
        Vec(col1 - 19,
        row2 + labelOffsetBig),
        "Fine");

    // FM
    addParam(SqHelper::createParamCentered<Rogan1PSBlue>(
        icomp,
        Vec(col2, row2),
        module, 
        Comp::FM_PARAM));
    addLabel(
        Vec(col2 - 15, row2 + labelOffsetBig),
        "FM");
}

void superWidget::addOtherKnobs(SuperModule *, std::shared_ptr<IComposite> icomp)
{
    // Detune
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col1, row3), 
        module, 
        Comp::DETUNE_PARAM));
    addLabel(
        Vec(col1 - 27, row3 + labelOffsetSmall),
        "Detune");

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(col1, row4), 
        module, 
        Comp::DETUNE_TRIM_PARAM));

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col2, row3), 
        module, 
        Comp::MIX_PARAM));
    addLabel(
        Vec(col2 - 18, row3 + labelOffsetSmall),
        "Mix");
    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(col2, row4), 
        module, 
        Comp::MIX_TRIM_PARAM));
}

const float jackX = 27;
const float jackDx = 33;
const float jackOffsetLabel = -30;
const float jackLabelPoints = 11;

void superWidget::addJacks(SuperModule *)
{
    Label* l = nullptr;
    // first row
    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX, jackRow1),
        module,
        Super<WidgetComposite>::DETUNE_INPUT));
    l = addLabel(
        Vec(jackX - 25, jackRow1 + jackOffsetLabel),
        "Detune");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 3 * jackDx, jackRow1),
        module,
        Super<WidgetComposite>::MIX_INPUT));
    l = addLabel(
        Vec(jackX + 3 * jackDx - 15, jackRow1 + jackOffsetLabel),
        "Mix");
    l->fontSize = jackLabelPoints;

    // second row
    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX, jackRow2),
        module,
        Super<WidgetComposite>::CV_INPUT));
    l = addLabel(
        Vec(jackX - 20, jackRow2 + jackOffsetLabel),
        "V/Oct");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 1 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::TRIGGER_INPUT));
    l = addLabel(
        Vec(jackX + 1 * jackDx - 17, jackRow2 + jackOffsetLabel),
        "Trig");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 2 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::FM_INPUT));
    l = addLabel(
        Vec(jackX + 2 * jackDx - 14, jackRow2 + jackOffsetLabel), "FM");
    l->fontSize = jackLabelPoints;

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jackX + 3 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::MAIN_OUTPUT));
    l = addLabel(
        Vec(jackX + 3 * jackDx - 18, jackRow2 + jackOffsetLabel),
        "Out", SqHelper::COLOR_WHITE);
    l->fontSize = jackLabelPoints;
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
superWidget::superWidget(SuperModule *module) : semitoneDisplay(module)
{
    setModule(module);
#else
superWidget::superWidget(SuperModule *module) :
    ModuleWidget(module),
    semitoneDisplay(module)
{
#endif
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/super_panel.svg");

    // Is this really needed?
    auto border = new PanelBorderWidget();
    border->box = box;
    addChild(border);

    addPitchKnobs(module, icomp);
    addOtherKnobs(module, icomp);
    addJacks(module);

    // the "classic" switch
    ToggleButton* tog = SqHelper::createParamCentered<ToggleButton>(
        icomp,
        Vec(83, 164),
        module,
        Comp::CLEAN_PARAM);
    tog->addSvg("res/clean-switch-01.svg");
    tog->addSvg("res/clean-switch-02.svg");
    tog->addSvg("res/clean-switch-03.svg");
    addParam(tog);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


#ifdef __V1
Model *modelSuperModule = createModel<SuperModule,
    superWidget>("squinkylabs-super");
#else
Model *modelSuperModule = Model::create<SuperModule,
    superWidget>("Squinky Labs",
    "squinkylabs-super",
    "Saws: super saw VCO emulation", OSCILLATOR_TAG);
#endif

#endif

