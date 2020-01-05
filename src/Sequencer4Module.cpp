
#include "Sequencer4Module.h"

#include <sstream>
#include "MidiSong4.h"
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SEQ4
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "seq/S4Button.h"

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

     std::function<void(bool isCtrlKey)> makeButtonHandler(int row, int column);
};


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

Sequencer4Widget::Sequencer4Widget(Sequencer4Module *module)
{
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    const float buttonSize = 50;
    const float buttonMargin = 10;
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        const float y = 70 + row * (buttonSize + buttonMargin);
        for (int col = 0; col < MidiSong4::numTracks; ++col) {
            const float x = 20 + col * (buttonSize + buttonMargin);
            S4Button* b = new S4Button(Vec(buttonSize, buttonSize), Vec(x, y));
            addChild(b);
            #if 0
            b->setHandler([](bool isCtrl) {
                DEBUG("click handled");
            });
            #endif

            b->setHandler(makeButtonHandler(row, col));
        }
    }
   

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

std::function<void(bool isCtrlKey)> Sequencer4Widget::makeButtonHandler(int row, int col)
{
    return [row, col, this](bool isCtrl) {
        DEBUG("click handled, r=%d c=%d", row, col);
    };
}

Model *modelSequencer4Module = createModel<Sequencer4Module, Sequencer4Widget>("squinkylabs-sequencer4");
#endif

