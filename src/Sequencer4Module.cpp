
#include "Sequencer4Module.h"

#include <sstream>
#include "MidiSong4.h"
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SEQ4
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqToggleLED.h"
#include "ctrl/SqWidgets.h"

#include "seq/ClockFinder.h"
#include "seq/S4Button.h"
#include "seq/SequencerSerializer.h"

#include "MidiSequencer4.h"

using Comp = Seq4<WidgetComposite>;

void Sequencer4Module::onSampleRateChange() {
}

Sequencer4Module::Sequencer4Module() {
    runStopRequested = false;
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    MidiSong4Ptr song = MidiSong4::makeTest(MidiTrack::TestContent::empty, 0);
    seq4 = MidiSequencer4::make(song);
    seq4Comp = std::make_shared<Comp>(this, song);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);

    onSampleRateChange();
    assert(seq4);
}

void Sequencer4Module::step() {
    //sequencer->undo->setModuleId(this->id);
    if (runStopRequested) {
        seq4Comp->toggleRunStop();
        runStopRequested = false;
    }
    seq4Comp->step();
}

MidiSong4Ptr Sequencer4Module::getSong() {
    return seq4Comp->getSong();
}

void Sequencer4Module::dataFromJson(json_t* data) {
    MidiSequencer4Ptr newSeq = SequencerSerializer::fromJson(data, this);
    setNewSeq(newSeq);
}

json_t* Sequencer4Module::dataToJson() {
    assert(seq4);
    return SequencerSerializer::toJson(seq4);
}

////////////////////
// module widget
////////////////////

struct Sequencer4Widget : ModuleWidget {
    Sequencer4Widget(Sequencer4Module*);
    void appendContextMenu(Menu* theMenu) override {
        ::rack::ui::MenuLabel* spacerLabel = new ::rack::ui::MenuLabel();
        theMenu->addChild(spacerLabel);
        ManualMenuItem* manual = new ManualMenuItem(
            "4X4 Manual",
            "https://github.com/squinkylabs/SquinkyVCV/blob/s45/docs/4x4.md");
        theMenu->addChild(manual);

#if 0 // doesn't work yet
        auto item = new SqMenuItem_BooleanParam2(module, Comp::TRIGGER_IMMEDIATE_PARAM);
        item->text = "Trigger Immediately";
        theMenu->addChild(item);
#endif
        auto item = new SqMenuItem( []() { return false; }, [this](){
           // float rawClockFalue = Comp::CLOCK_INPUT_PARAM
            float rawClockValue = ::rack::appGet()->engine->getParam(module, Comp::CLOCK_INPUT_PARAM);
            SeqClock::ClockRate rate =  SeqClock::ClockRate(int(std::round(rawClockValue)));
            const int div = SeqClock::clockRate2Div(rate);
            ClockFinder::go(this, div, Comp::CLOCK_INPUT, Comp::RUN_INPUT, Comp::RESET_INPUT, false);
        });
        item->text = "Hookup Clock";
        theMenu->addChild(item);
        //ClockFinder::updateMenu(theMenu);
    }

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_GREY) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    Label* addLabelLeft(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_GREY) {
        Label* label = new Label();
        label->alignment = Label::LEFT_ALIGNMENT;
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void setNewSeq(MidiSequencer4Ptr newSeq);
    void addControls(Sequencer4Module* module,
                     std::shared_ptr<IComposite> icomp);
    void addBigButtons(Sequencer4Module* module);
    void addJacks(Sequencer4Module* module);
    void toggleRunStop(Sequencer4Module* module);
    S4ButtonGrid buttonGrid;
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

Sequencer4Widget::Sequencer4Widget(Sequencer4Module* module) {
    setModule(module);
    if (module) {
        module->widget = this;
    }
    box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/sq4_panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addControls(module, icomp);
    addBigButtons(module);
    addJacks(module);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void Sequencer4Widget::setNewSeq(MidiSequencer4Ptr newSeq) {
    buttonGrid.setNewSeq(newSeq);
}

void Sequencer4Widget::toggleRunStop(Sequencer4Module* module) {
    module->toggleRunStop();
}

#define _LAB
void Sequencer4Widget::addControls(Sequencer4Module* module,
                                   std::shared_ptr<IComposite> icomp) {
    const float controlX = 20 - 6;

    float y = 20;
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
    p->box.size.x = 85 + 8;  // width
    p->box.size.y = 22;      // should set auto like button does
    p->text = "x64";
    p->setLabels(Comp::getClockRates());
    addParam(p);
    y += 42;
    for (int i = 0; i < 4; ++i) {
#if defined(_LAB) && false
        addLabelLeft(Vec(controlX - 4, y),
                     "Polyphony");
#endif
        p = SqHelper::createParam<PopupMenuParamWidget>(
            icomp,
            Vec(controlX + 52, y),  // 54 too much 50 too little
            module,
            Comp::NUM_VOICES0_PARAM + i);
        p->text = "4";              // default text for the module browser
        p->box.size.x = 40;  // width
        p->box.size.y = 22;      // should set auto like button does
        p->setLabels(Comp::getPolyLabels());
        addParam(p);

        y += S4ButtonGrid::buttonMargin + S4ButtonGrid::buttonSize;
    }

    y += -20;
    //   const float yy = y;
#ifdef _LAB
    addLabel(Vec(controlX - 8, y),
             "Run");
#endif
    y += 20;

    float controlDx = 0;

    // run/stop buttong
    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(controlX + controlDx, y),
        module,
        Comp::RUN_STOP_LIGHT));
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    tog->setHandler([this, module](bool ctrlKey) {
        this->toggleRunStop(module);
    });
    addChild(tog);

    // add a hidden running control, just so ClockFinder can find it
    auto runWidget = SqHelper::createParam<NullWidget>(
        icomp,
        Vec(0, 0),
        module,
        Comp::RUNNING_PARAM);
    runWidget->box.size.x = 0;
    runWidget->box.size.y = 0;
    addParam(runWidget);
}

void Sequencer4Widget::addBigButtons(Sequencer4Module* module) {
    if (module) {
        buttonGrid.init(this, module, module->getSong(), module->seq4Comp);
    } else {
        WARN("make the module browser draw the buttons");
        
         buttonGrid.init(this, nullptr, nullptr, nullptr);
    }
}

void Sequencer4Widget::addJacks(Sequencer4Module* module) {
    const float jacksY1 = 340;
    //  const float jacksY2 = 330+2;
    const float jacksDx = 40;
    const float jacksX = 140;
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
}

void Sequencer4Module::setNewSeq(MidiSequencer4Ptr newSeq) {
    MidiSong4Ptr oldSong = seq4->song;
    seq4 = newSeq;

    if (widget) {
        widget->setNewSeq(newSeq);
    }

    {
        // Must lock the songs when swapping them or player
        // might glitch (or crash).
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(seq4->song->lock);
        seq4Comp->setSong(seq4->song);
    }
}

Model* modelSequencer4Module = createModel<Sequencer4Module, Sequencer4Widget>("squinkylabs-sequencer4");
#endif
