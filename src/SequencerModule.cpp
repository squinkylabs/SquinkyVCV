
#include <iostream>
#include "Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"
#include "seq/NoteDisplay.h"
#include "seq/AboveNoteGrid.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/ToggleButton.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqToggleLED.h"

#include "seq/SequencerSerializer.h"
#include "MidiLock.h"
#include "MidiSong.h"
#include "TimeUtils.h"

#include "SequencerModule.h"

using Comp = Seq<WidgetComposite>;

#ifdef __V1x
SequencerModule::SequencerModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
#else
SequencerModule::SequencerModule()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS)
{
#endif
    runStopRequested = false;
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    sequencer = MidiSequencer::make(song);
    seqComp = std::make_shared<Comp>(this, song);
}

static const char* helpUrl = "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/sq.md";

struct SequencerWidget : ModuleWidget
{
    SequencerWidget(SequencerModule *);
    DECLARE_MANUAL(helpUrl);

    /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void step() override;

    NoteDisplay* noteDisplay = nullptr;
    AboveNoteGrid* headerDisplay = nullptr;
    ToggleButton*  scrollControl = nullptr;
    SequencerModule* _module = nullptr;

    void addJacks(SequencerModule *module);
    void addControls(SequencerModule *module, std::shared_ptr<IComposite> icomp);
    void toggleRunStop(SequencerModule *module);
};

void SequencerWidget::step()
 {
    ModuleWidget::step();
    if (scrollControl && _module && _module->isRunning()) {
        
        const int y = scrollControl->getValue();
        if (y) {
            float curTime = _module->getPlayPosition();
            if (y == 2) {
                auto curBar = TimeUtils::time2bar(curTime);
                curTime = TimeUtils::bar2time(curBar);
            }
            auto seq = _module->getSeq();
            seq->editor-> advanceCursorToTime(curTime, false);
        }
    }
}

void SequencerWidget::toggleRunStop(SequencerModule *module)
{
    module->toggleRunStop();
}

void sequencerHelp()
{
    SqHelper::openBrowser(helpUrl);
}

#ifdef __V1x
SequencerWidget::SequencerWidget(SequencerModule *module) : _module(module)
{
    setModule(module);

#else
SequencerWidget::SequencerWidget(SequencerModule *module) : ModuleWidget(module), _module(module)
{
#endif
    if (module) {
        module->widget = this;
    }
    // was 14, before 8
    // 8 for panel, 28 for notes
    const int panelWidthHP = 8;
    const int width = (panelWidthHP + 28) * RACK_GRID_WIDTH; 
    box.size = Vec(width, RACK_GRID_HEIGHT);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setPanel(this, "res/seq_panel.svg");
    box.size.x = width;     // restore to the full width that we want to be
    {
        const float topDivider = 60;
        const float x = panelWidthHP * RACK_GRID_WIDTH;
        const float width = 28 * RACK_GRID_WIDTH;
        const Vec notePos = Vec(x, topDivider);
        const Vec noteSize = Vec(width, RACK_GRID_HEIGHT - topDivider);

        const Vec headerPos = Vec(x, 0);
        const Vec headerSize = Vec(width, topDivider);

        MidiSequencerPtr seq;
        if (module) {
            seq = module->sequencer;
        }
        headerDisplay = new AboveNoteGrid(headerPos, headerSize, seq);
        noteDisplay = new NoteDisplay(notePos, noteSize, seq);
        addChild(noteDisplay);
        addChild(headerDisplay);
    }

    addControls(module, icomp);
    addJacks(module);
 
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void SequencerWidget::addControls(SequencerModule *module, std::shared_ptr<IComposite> icomp)
{
    const float controlX = 20;
    addParam(SqHelper::createParam<Rogan2PSBlue>(
        icomp,
        Vec(controlX, 65),
        module,
        Comp::TEMPO_PARAM));
    addLabel(Vec(60, 40), "Tempo");

    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(controlX, 120),
        module,
        Comp::CLOCK_INPUT_PARAM);
    p->box.size.x = 85;    // width
    p->box.size.y = 22;     // should set auto like button does
    p->setLabels(Comp::getClockRates());
    addParam(p);


    SqToggleLED* tog = (createLight<SqToggleLED>(
        Vec(controlX, 180),
        module,
        Seq<WidgetComposite>::RUN_STOP_LIGHT));
    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    tog->setHandler( [this, module](bool ctrlKey) {
        this->toggleRunStop(module);
    });
    addChild(tog);
    addLabel(
        Vec(controlX+30, 180),
        "Run");

    {
    scrollControl = SqHelper::createParam<ToggleButton>(
        icomp,
        Vec(controlX, 220),
        module,
        Comp::PLAY_SCROLL_PARAM);
    scrollControl->addSvg("res/square-button-01.svg");
    scrollControl->addSvg("res/square-button-02.svg");
    addParam(scrollControl);

    addLabel(
        Vec(controlX + 30, 220),
        "Scroll");
    }
}

void SequencerWidget::addJacks(SequencerModule *module)
{
    const float jacksY = 310;
    const float jacksDy = 40;
    const float jacksLabelY = 280;
    const float jacksDx = 40;
    const float jacksX = 20;
    const float labelX = jacksX - 20;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY),
        module,
        Comp::CLOCK_INPUT));
    addLabel(
        Vec(5 + labelX + 0 * jacksDx, jacksLabelY),
        "Clk");

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY),
        module,
        Comp::RESET_INPUT));
    addLabel(
        Vec(-5 + labelX + 1 * jacksDx, jacksLabelY),
        "Reset");

    addInput(createInputCentered<PJ301MPort>(
        Vec(jacksX + 2 * jacksDx, jacksY),
        module,
        Comp::RUN_INPUT));
    addLabel(
        Vec(labelX + 2 * jacksDx, jacksLabelY),
        "Run");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jacksX, jacksY + jacksDy),
        module,
        Seq<WidgetComposite>::CV_OUTPUT));
    addLabel(
        Vec(labelX, jacksLabelY + jacksDy),
        "CV");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY + jacksDy),
        module,
        Seq<WidgetComposite>::GATE_OUTPUT));
    addLabel(
        Vec(labelX + 1 * jacksDx, jacksLabelY + jacksDy),
        "Gate");

    addChild(createLight<MediumLight<GreenLight>>(
        Vec(jacksX + 2 * jacksDx , jacksY + jacksDy),
        module,
        Seq<WidgetComposite>::GATE_LIGHT));
}

#ifdef __V1x
void SequencerModule::dataFromJson(json_t *data)
#else
void SequencerModule::fromJson(json_t* data)
#endif
{
    MidiSequencerPtr newSeq = SequencerSerializer::fromJson(data);
    setNewSeq(newSeq);
}

void SequencerModule::setNewSeq(MidiSequencerPtr newSeq)
{
    MidiSongPtr oldSong = sequencer->song;
    sequencer = newSeq;
    if (widget) {
        widget->noteDisplay->setSequencer(newSeq);
        widget->headerDisplay->setSequencer(newSeq);
    }

    {
        // Must lock the songs when swapping them or player 
        // might glitch (or crash).
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(sequencer->song->lock);
        seqComp->setSong(sequencer->song);
    }
}


void SequencerModule::onReset()
{
    Module::onReset();
    std::shared_ptr<MidiSong> newSong = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    MidiSequencerPtr newSeq  = MidiSequencer::make(newSong);
    setNewSeq(newSeq);
}


// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.

#ifdef __V1x
Model *modelSequencerModule = createModel<SequencerModule, SequencerWidget>("squinkylabs-sequencer");
#else
Model *modelSequencerModule = Model::create<SequencerModule, SequencerWidget>("Squinky Labs",
    "squinkylabs-sequencer",
    "S", SEQUENCER_TAG);
#endif
#endif