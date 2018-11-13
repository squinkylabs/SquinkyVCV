
#include <iostream>
#include "Squinky.hpp"

#ifdef _SEQ
#include "WidgetComposite.h"
#include "Seq.h"
#include "widgets.hpp"


struct SequencerModule : Module
{
    SequencerModule();
    Seq<WidgetComposite> seq;
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


struct NoteDisplay : OpaqueWidget {
    SequencerModule *module;

    void draw(NVGcontext *vg) override 
    {
        // draw some squares for fun
       // nvgScale(vg, 2, 2);
        nvgFillColor(vg, nvgRGBA(0xff, 0x00, 0x00, 0xff));
        nvgBeginPath(vg);
        nvgRect(vg, 50, 50, 30, 30);
		nvgFill(vg);

         nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0xff, 0xff));
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, 30, 30);
		nvgFill(vg);
    }
    void onMouseDown(EventMouseDown &e) override
    {
        OpaqueWidget::onMouseDown(e);
        std::cout << "onMouseDown " << e.button << std::flush << std::endl;
    }
	void onMouseMove(EventMouseMove &e) override
    {
       //  std::cout << "onMouseMove " << std::flush << std::endl;
	    OpaqueWidget::onMouseMove(e);
    }
	void onFocus(EventFocus &e) override
    {
         std::cout << "onFocus " << std::flush << std::endl;
       // OpaqueWidget::onFocus(e);
       e.consumed = true;
    }
	void onText(EventText &e) override
    {
        std::cout << "onText " <<  std::flush << std::endl;
        OpaqueWidget::onText(e);
    }

	void onKey(EventKey &e) override
    {
        std::cout << "onKey " << e.key << std::flush << std::endl;
        if (!e.consumed) {
		    OpaqueWidget::onKey(e);
	    }
    }
    void onMouseEnter(EventMouseEnter &e) override
     {
         std::cout << "nmouseenger "  << std::flush << std::endl;
     }
	/** Called when another widget begins responding to `onMouseMove` events */
//	virtual void onMouseLeave(EventMouseLeave &e) {}
	//virtual void onFocus(EventFocus &e) {}
	void onDefocus(EventDefocus &e) override
    
     {
        std::cout << "defoucs "  << std::flush << std::endl;
        e.consumed=true;
     }
};

                                  //SequencerModule
 SequencerWidget::SequencerWidget(SequencerModule *module) : ModuleWidget(module)
{
    const int width = (14 + 0) * RACK_GRID_WIDTH;      // 14 for panel, 14 for notes
    box.size = Vec(width, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }
    #if 0
	{
		NoteDisplay *display = new NoteDisplay();
		display->module = module;
		display->box.pos = Vec( 14 * RACK_GRID_WIDTH, 0);
		display->box.size = Vec(14 * RACK_GRID_WIDTH,RACK_GRID_HEIGHT);
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
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelSequencerModule = Model::create<SequencerModule, SequencerWidget>("Squinky Labs",
    "squinkylabs-sequencer",
    "S", SEQUENCER_TAG);
#endif