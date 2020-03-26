#pragma once

////////////////////
// module widget
////////////////////

#include "ctrl/SqHelper.h"

namespace rack {
    namespace app {
        struct ModuleWidget;
    }
}

using ModuleWidget =  ::rack::app::ModuleWidget;
using Menu = ::rack::ui::Menu;
using Label = ::rack::ui::Label;
using Vec = ::rack::math::Vec;

class S4ButtonGrid;
class Sequencer4Module;

struct Sequencer4Widget : ModuleWidget {
    Sequencer4Widget(Sequencer4Module*);
    void appendContextMenu(Menu* theMenu) override ;
    std::shared_ptr<S4ButtonGrid> getButtonGrid() { return buttonGrid; }

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
    std::shared_ptr<S4ButtonGrid> buttonGrid;
};
