
#include <iostream>
#include "Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"
//#include "widgets.hpp"
#include "seq/NoteDisplay.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/PopupMenuParamWidget.h"
#include "seq/SequencerSerializer.h"
#include "MidiLock.h"
#include "MidiSong.h"

using Comp = Seq<WidgetComposite>;
class SequencerWidget;

struct SequencerModule : Module
{
    SequencerModule();
    std::shared_ptr<Seq<WidgetComposite>> seqComp;

    MidiSequencerPtr sequencer;
    SequencerWidget* widget = nullptr;
#ifndef __V1
    json_t *toJson() override
    {
        assert(sequencer);
        return SequencerSerializer::toJson(sequencer);
    }
    void fromJson(json_t* data) override;
#endif

    void step() override
    {
        seqComp->step();
    }

    void stop()
    {
        seqComp->stop();
    }
};

#ifdef __V1
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
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    //sequencer = std::make_shared<MidiSequencer>(song);
    //sequencer->makeEditor();
    sequencer = MidiSequencer::make(song);
    seqComp = std::make_shared<Comp>(this, song);
}

struct SequencerWidget : ModuleWidget
{
    SequencerWidget(SequencerModule *);
    DECLARE_MANUAL("https://github.com/squinkylabs/SquinkyVCV/blob/sq3/docs/sq.md");
    
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

    NoteDisplay* noteDisplay = nullptr;
};

#ifdef __V1
SequencerWidget::SequencerWidget(SequencerModule *module)
{
    setModule(module);

#else
SequencerWidget::SequencerWidget(SequencerModule *module) : ModuleWidget(module)
{
#endif
    if (module) {
        module->widget = this;
    }
    const int width = (14 + 28) * RACK_GRID_WIDTH;      // 14 for panel, other for notes
    box.size = Vec(width, RACK_GRID_HEIGHT);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setPanel(this, "res/blank_panel.svg");
	{
        const Vec notePos = Vec( 14 * RACK_GRID_WIDTH, 0);
        const Vec noteSize =Vec(28 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
       // module->stop();         // don't start playback immediately

        MidiSequencerPtr seq;
        if (module) {
            seq = module->sequencer;
        }
		noteDisplay = new NoteDisplay(notePos, noteSize, seq);
		addChild(noteDisplay);
	}

    addInput(createInputCentered<PJ301MPort>(
        Vec(50, 40),
        module,
        Comp::CLOCK_INPUT));
    addLabel(Vec(35, 56), "Clk");
#ifndef __V1
    PopupMenuParamWidget* p = PopupMenuParamWidget::create<PopupMenuParamWidget>(
        Vec (40, 90),
        module, 
        Comp::CLOCK_INPUT_PARAM,
        0, 5, 2);
    p->box.size.x  = 100;    // width
    p->setLabels(Comp::getClockRates());
    addParam(p);
#endif


    addParam(SqHelper::createParam<Rogan2PSBlue>(
        icomp,
        Vec(60, 150),
        module,
        Comp::TEMPO_PARAM));
    addLabel(Vec(60, 200), "Tempo");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(50, 339),
        module,
        Seq<WidgetComposite>::CV_OUTPUT));
    addLabel(Vec(35, 310), "CV");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(90, 339),
        module,
        Seq<WidgetComposite>::GATE_OUTPUT));
    addLabel(Vec(75, 310), "G");

    addChild(createLight<MediumLight<GreenLight>>(
        Vec(120, 310), module,  Seq<WidgetComposite>::GATE_LIGHT));

      // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 

}

#ifndef __V1
void SequencerModule::fromJson(json_t* data) 
{
    MidiSongPtr oldSong = sequencer->song;

    MidiSequencerPtr newSeq = SequencerSerializer::fromJson(data);
    sequencer = newSeq;
    if (widget) {
        widget->noteDisplay->setSequencer(newSeq);
    }

    {
        // Must lock the songs when swapping them or player 
        // might glitch (or crash).
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(sequencer->song->lock);
        seqComp->setSong(sequencer->song);
    }
}
#endif
// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.

#ifdef __V1
Model *modelSequencerModule = createModel<SequencerModule, SequencerWidget>("seq");
#else
Model *modelSequencerModule = Model::create<SequencerModule, SequencerWidget>("Squinky Labs",
    "squinkylabs-sequencer",
    "S", SEQUENCER_TAG);
#endif
#endif