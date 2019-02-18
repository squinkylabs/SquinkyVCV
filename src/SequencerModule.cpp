
#include <iostream>
#include "Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"
#include "widgets.hpp"
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

    json_t *toJson() override
    {
        assert(sequencer);
        return SequencerSerializer::toJson(sequencer);
    }
    void fromJson(json_t* data) override;

    void step() override
    {
        seqComp->step();
    }

    void stop()
    {
        seqComp->stop();
    }
};

SequencerModule::SequencerModule()
    : Module(Comp::NUM_PARAMS,
        Comp::NUM_INPUTS,
        Comp::NUM_OUTPUTS,
        Comp::NUM_LIGHTS)
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    //sequencer = std::make_shared<MidiSequencer>(song);
    //sequencer->makeEditor();
    sequencer = MidiSequencer::make(song);
    seqComp = std::make_shared<Comp>(this, song);
}

struct SequencerWidget : ModuleWidget
{
    SequencerWidget(SequencerModule *);
    Menu* createContextMenu() override;

        /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
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

inline Menu* SequencerWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/sq3/docs/sq.md");
    theMenu->addChild(manual);
    return theMenu;
}

 SequencerWidget::SequencerWidget(SequencerModule *module) : ModuleWidget(module)
{
    if (module) {
        module->widget = this;
    }
    const int width = (14 + 28) * RACK_GRID_WIDTH;      // 14 for panel, other for notes
    box.size = Vec(width, RACK_GRID_HEIGHT);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(pluginInstance, "res/blank_panel.svg")));
        addChild(panel);
    }

	{
        const Vec notePos = Vec( 14 * RACK_GRID_WIDTH, 0);
        const Vec noteSize =Vec(28 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
       // module->stop();         // don't start playback immediately
		noteDisplay = new NoteDisplay(notePos, noteSize, module->sequencer);
		addChild(noteDisplay);
	}

    addInput(createInputCentered<PJ301MPort>(
        Vec(50, 40),
        module,
        Comp::CLOCK_INPUT));
    addLabel(Vec(35, 56), "Clk");

    PopupMenuParamWidget* p = PopupMenuParamWidget::create<PopupMenuParamWidget>(
        Vec (40, 90),
        module, 
        Comp::CLOCK_INPUT_PARAM,
        0, 5, 2);
    p->box.size.x  = 100;    // width
    p->setLabels(Comp::getClockRates());
    addParam(p);


    addParam(ParamWidget::create<Rogan2PSBlue>(
        Vec(60, 150),
        module,
        Comp::TEMPO_PARAM,
        40, 200, 120));
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

    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(120, 310), module,  Seq<WidgetComposite>::GATE_LIGHT));

      // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 

}


void SequencerModule::fromJson(json_t* data) 
{
    MidiSongPtr oldSong = sequencer->song;

    MidiSequencerPtr newSeq = SequencerSerializer::fromJson(data);
    sequencer = newSeq;
    if (widget) {
        printf("sending to note display\n");
        widget->noteDisplay->setSequencer(newSeq);
    }

#if 0
    printf("after deserialze, using seq %p, song %p, pk0 %p\n",
        sequencer.get(),
        sequencer->song.get(),
        sequencer->song->getTrack(0).get());
    printf("midi context = %p\n", sequencer->context.get());
    fflush(stdout);
    printf("midi context cursor = %f\n", sequencer->context->cursorPitch());
    fflush(stdout);
#endif

    {
        // Must lock the songs when swapping them or player 
        // might glitch (or crash).
        MidiLocker oldL(oldSong->lock);
        MidiLocker newL(sequencer->song->lock);
        seqComp->setSong(sequencer->song);
    }
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelSequencerModule = Model::create<SequencerModule, SequencerWidget>("Squinky Labs",
    "squinkylabs-sequencer",
    "S", SEQUENCER_TAG);
#endif