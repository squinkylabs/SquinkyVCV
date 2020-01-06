
#include "Sequencer4Module.h"

#include <sstream>
#include "MidiSong4.h"
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SEQ4
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "seq/S4Button.h"
#include "ctrl/SqToggleLED.h"

using Comp = Seq4<WidgetComposite>;

void Sequencer4Module::onSampleRateChange()
{
}

Sequencer4Module::Sequencer4Module()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    MidiSong4Ptr song = MidiSong4::makeTest(MidiTrack::TestContent::empty, 0);
    seq4Comp = std::make_shared<Comp>(this, song);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
   // seq4Comp->init();
}

void Sequencer4Module::step()
{
    seq4Comp->step();
}

////////////////////
// module widget
////////////////////

struct Sequencer4Widget : ModuleWidget
{
    Sequencer4Widget(Sequencer4Module *);
    DECLARE_MANUAL("Blank Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    Label* addLabelLeft(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->alignment = Label::LEFT_ALIGNMENT;
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addControls(Sequencer4Module *module,
        std::shared_ptr<IComposite> icomp);
    void addBigButtons();
    void addJacks(Sequencer4Module *module);
    void toggleRunStop(Sequencer4Module *module);
    //void onButton(const event::Button &e) override;

    std::function<void(bool isCtrlKey)> makeButtonHandler(int row, int column);
    std::function<void()> makePasteHandler(int row, int column);
};

#if 0
void Sequencer4Widget::onButton(const event::Button &e)
{
    ModuleWidget::onButton(e);
}
#endif

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

Sequencer4Widget::Sequencer4Widget(Sequencer4Module *module)
{
    setModule(module);
    box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/sq4_panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addControls(module, icomp);
    addBigButtons();
    addJacks(module);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void Sequencer4Widget::toggleRunStop(Sequencer4Module *module)
{
    module->toggleRunStop();
}

#define _LAB
void Sequencer4Widget::addControls(Sequencer4Module *module,
        std::shared_ptr<IComposite> icomp)
{
  const float controlX = 20 - 6;

    float y = 50;
#ifdef _LAB
    addLabelLeft(Vec(controlX - 4, y),
        "Clock rate");
#endif
    y += 20;
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(controlX, y),
        module,
        Comp::CLOCK_INPUT_PARAM);
    p->box.size.x = 85 + 8;    // width
    p->box.size.y = 22;         // should set auto like button does
    p->setLabels(Comp::getClockRates());
    addParam(p);

    y += 28;
#ifdef _LAB
    addLabelLeft(Vec(controlX - 4, y),
        "Polyphony");
#endif
    y += 20;
    p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(controlX, y),
        module,
        Comp::NUM_VOICES_PARAM);
    p->box.size.x = 85 + 8;     // width
    p->box.size.y = 22;         // should set auto like button does
    p->setLabels(Comp::getPolyLabels());
    addParam(p);
   
    y += 28;
    const float yy = y;
#ifdef _LAB
    addLabel(Vec(controlX - 8, y),
        "Run");
#endif
    y += 20;

    float controlDx = 34;

        // run/stop buttong
    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(controlX + controlDx, y),
        module,
        Comp::RUN_STOP_LIGHT));
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    tog->setHandler( [this, module](bool ctrlKey) {
        this->toggleRunStop(module);
    });
    addChild(tog);
}

void Sequencer4Widget::addBigButtons()
{
    const float buttonSize = 50;
    const float buttonMargin = 10;
    const float jacksX = 380;
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        const float y = 70 + row * (buttonSize + buttonMargin);
        for (int col = 0; col < MidiSong4::numTracks; ++col) {
            const float x = 130 + col * (buttonSize + buttonMargin);
            S4Button* b = new S4Button(Vec(buttonSize, buttonSize), Vec(x, y));
            addChild(b);
            b->setClickHandler(makeButtonHandler(row, col));
            b->setPasteHandler(makePasteHandler(row, col));
        }

        const float jacksY = y + 8;
        const float jacksDy = 28;
        
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(jacksX, jacksY),
            module,
            Comp::CV0_OUTPUT + row));
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(jacksX, jacksY + jacksDy),
            module,
            Comp::GATE0_OUTPUT + row));
    }
}

void Sequencer4Widget::addJacks(Sequencer4Module *module)
{
    const float jacksY1 = 286-2;
    const float jacksY2 = 330+2;
    const float jacksDx = 40;
    const float jacksX = 20;
#ifdef _LAB
    const float labelX = jacksX - 20;
    const float dy = -32;
#endif

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY1),
        module,
        Comp::CLOCK_INPUT));
#ifdef _LAB
    addLabel(
        Vec(3 + labelX + 0 * jacksDx, jacksY1 + dy),
        "Clk");
#endif

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY1),
        module,
        Comp::RESET_INPUT));
#ifdef _LAB
    addLabel(
        Vec(-4 + labelX + 1 * jacksDx, jacksY1 + dy),
        "Reset");
#endif

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 2 * jacksDx, jacksY1),
        module,
        Comp::RUN_INPUT));
#ifdef _LAB
    addLabel(
        Vec(labelX + 1 + 2 * jacksDx, jacksY1 + dy),
        "Run");
#endif

#if 0
    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jacksX, jacksY2),
        module,
        Comp::CV_OUTPUT));
#ifdef _LAB
    addLabel(
        Vec(labelX+2, jacksY2 + dy),
        "CV");
#endif

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY2),
        module,
        Comp::GATE_OUTPUT));
#ifdef _LAB
    addLabel(
        Vec(labelX + 1 * jacksDx, jacksY2 + dy),
        "Gate");
#endif
    addChild(createLight<MediumLight<GreenLight>>(
        Vec(jacksX + 2 * jacksDx -6 , jacksY2 -6),
        module,
        Comp::GATE_LIGHT));
#endif
}


std::function<void(bool isCtrlKey)> Sequencer4Widget::makeButtonHandler(int row, int col)
{
    return [row, col, this](bool isCtrl) {
        DEBUG("NIMP click handled, r=%d c=%d", row, col);
    };
}

std::function<void()> Sequencer4Widget::makePasteHandler(int row, int col)
{
    return [row, col, this]() {
        DEBUG("MINP paste handled, r=%d c=%d", row, col);
    };
}

Model *modelSequencer4Module = createModel<Sequencer4Module, Sequencer4Widget>("squinkylabs-sequencer4");
#endif

