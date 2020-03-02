
#include "Squinky.hpp"
#include "FrequencyShifter.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"

#ifdef _BOOTY
#include "DrawTimer.h"

#ifdef _TIME_DRAWING
static DrawTimer drawTimer("Booty");
#endif

using Comp = FrequencyShifter<WidgetComposite>;

/**
 * Implementation class for BootyModule
 */
struct BootyModule : Module
{
    BootyModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
#ifdef __V1x
    virtual json_t *dataToJson() override;
    virtual void dataFromJson(json_t *root) override;
#else
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
#endif

    void onSampleRateChange() override;

    std::shared_ptr<Comp> shifter;
private:
    typedef float T;
public:
    ChoiceButton * rangeChoice;
};

extern float values[];
extern const char* ranges[];

#ifdef __V1x
BootyModule::BootyModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    shifter = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
#else
BootyModule::BootyModule() :
    Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS),
    shifter(std::make_shared<Comp>(this))
{
#endif
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    shifter->init();
}

void BootyModule::onSampleRateChange()
{
    T rate = SqHelper::engineGetSampleRate();
    shifter->setSampleRate(rate);
}

#ifdef __V1x
json_t *BootyModule::dataToJson()
#else
json_t *BootyModule::toJson()
#endif
{
    json_t *rootJ = json_object();
    const int rg = shifter->freqRange;
    json_object_set_new(rootJ, "range", json_integer(rg));
    return rootJ;
}

#ifdef __V1x
void BootyModule::dataFromJson(json_t *rootJ)
#else
void BootyModule::fromJson(json_t *rootJ)
#endif
{
    json_t *driverJ = json_object_get(rootJ, "range");
    if (driverJ) {
        const int rg = json_number_value(driverJ);

        // TODO: should we be more robust about float <> int issues?
        //need to tell the control what text to display
        for (int i = 0; i < 5; ++i) {
            if (rg == values[i]) {
                rangeChoice->text = ranges[i];
            }
        }
        shifter->freqRange = rg;
    }
}

void BootyModule::step()
{
    shifter->step();
}

/***********************************************************************************
 *
 * RangeChoice selector widget
 *
 ***********************************************************************************/

const char* ranges[5] = {
    "5 Hz",
    "50 Hz",
    "500 Hz",
    "5 kHz",
    "exp"
};

float values[5] = {
    5,
    50,
    500,
    5000,
    0
};

struct RangeItem : MenuItem
{
    RangeItem(int index, float * output, ChoiceButton * inParent) :
        rangeIndex(index), rangeOut(output), rangeChoice(inParent)
    {
        text = ranges[index];
    }

    const int rangeIndex;
    float * const rangeOut;
    ChoiceButton* const rangeChoice;

#ifdef __V1x
    void onAction(const event::Action &e) override
#else
    void onAction(EventAction &e) override
#endif
    {
        rangeChoice->text = ranges[rangeIndex];
        *rangeOut = values[rangeIndex];
    }
};

struct RangeChoice : ChoiceButton
{
    RangeChoice(float * out, const Vec& pos, float width) : output(out)
    {
        assert(*output == 5);
        this->text = std::string(ranges[0]);
        this->box.pos = pos;
        this->box.size.x = width;
    }
    float * const output;
#ifdef __V1x
    void onAction(const event::Action &e) override
    {
        Menu* menu = createMenu();
#else
    void onAction(EventAction &e) override
    {
        Menu *menu = gScene->createMenu();
#endif

        menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
        menu->box.size.x = box.size.x;
        {
            menu->addChild(new RangeItem(0, output, this));
            menu->addChild(new RangeItem(1, output, this));
            menu->addChild(new RangeItem(2, output, this));
            menu->addChild(new RangeItem(3, output, this));
            menu->addChild(new RangeItem(4, output, this));
        }
    }
};

////////////////////
// module widget
////////////////////

struct BootyWidget : ModuleWidget
{
    BootyWidget(BootyModule *);
    DECLARE_MANUAL("Booty Shifter manual", "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/shifter.md");

#ifdef _TIME_DRAWING
    // Booty: avg = 30.679601, stddev = 10.568395 (us) Quota frac=0.184078
    void draw(const DrawArgs &args) override
    {
        DrawLocker l(drawTimer);
        ModuleWidget::draw(args);
    }
#endif
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1x
BootyWidget::BootyWidget(BootyModule *module)
{
    setModule(module);
#else
BootyWidget::BootyWidget(BootyModule *module) : ModuleWidget(module)
{
#endif
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/booty_panel.svg");

    const int leftInputX = 11;
    const int rightInputX = 55;

    const int row0 = 45;
    const int row1 = 102;
    static int row2 = 186;

    // Inputs on Row 0
    addInput(createInput<PJ301MPort>(
        Vec(leftInputX, row0),
        module,
        Comp::AUDIO_INPUT));
    addInput(createInput<PJ301MPort>(
        Vec(rightInputX, row0),
        module,
        Comp::CV_INPUT));

    // shift Range on row 2
    const float margin = 16;
    float xPos = margin;
    float width = 6 * RACK_GRID_WIDTH - 2 * margin;

    // TODO: why do we need to reach into the module from here? I guess any
    // time UI callbacks need to go bak..
    if (module) {
        module->rangeChoice = new RangeChoice(&module->shifter->freqRange, Vec(xPos, row2), width);
        addChild(module->rangeChoice);
    } else {
        // let's make one, just for the module browser
        static float dummyRange = 5;
        auto displayGizmo = new RangeChoice(&dummyRange, Vec(xPos, row2), width);
        addChild(displayGizmo);
    }

    // knob on row 1
    addParam(SqHelper::createParam<Rogan3PSBlue>(
        icomp,
        Vec(18, row1),
        module,
        Comp::PITCH_PARAM));

    const float row3 = 317.5;

    // Outputs on row 3
    const float leftOutputX = 9.5;
    const float rightOutputX = 55.5;

    addOutput(createOutput<PJ301MPort>(
        Vec(leftOutputX, row3),
        module,
        Comp::SIN_OUTPUT));
    addOutput(createOutput<PJ301MPort>(
        Vec(rightOutputX, row3),
        module,
        Comp::COS_OUTPUT));

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
#ifdef __V1x
Model *modelBootyModule = createModel<BootyModule, BootyWidget>("squinkylabs-freqshifter");
#else
Model *modelBootyModule = Model::create<BootyModule, BootyWidget>("Squinky Labs",
    "squinkylabs-freqshifter",
    "Booty Shifter: Frequency Shifter", EFFECT_TAG, RING_MODULATOR_TAG);
#endif
#endif
