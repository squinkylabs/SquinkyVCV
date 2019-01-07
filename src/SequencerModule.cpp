
#include <iostream>
#include "Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"
#include "widgets.hpp"
#include "NoteDisplay.h"


struct SequencerModule : Module
{
    SequencerModule();
    Seq<WidgetComposite> seq;

    void step() override
    {
        seq.step();
    }
};

SequencerModule::SequencerModule()
    : Module(seq.NUM_PARAMS,
    seq.NUM_INPUTS,
    seq.NUM_OUTPUTS,
    seq.NUM_LIGHTS),
    seq(this)
{
}


struct SequencerWidget : ModuleWidget
{
    SequencerWidget(SequencerModule *);

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
};

 SequencerWidget::SequencerWidget(SequencerModule *module) : ModuleWidget(module)
{
    const int width = (14 + 14) * RACK_GRID_WIDTH;      // 14 for panel, 14 for notes
    box.size = Vec(width, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }
    #if 1
	{
        const Vec notePos = Vec( 14 * RACK_GRID_WIDTH, 0);
        const Vec noteSize =Vec(14 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
		NoteDisplay *display = new NoteDisplay(notePos, noteSize);
		addChild(display);
	}
    #endif

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
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelSequencerModule = Model::create<SequencerModule, SequencerWidget>("Squinky Labs",
    "squinkylabs-sequencer",
    "S", SEQUENCER_TAG);
#endif