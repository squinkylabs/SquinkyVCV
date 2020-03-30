
#include "../Squinky.hpp"
#include "MidiSequencer4.h"
#include "S4ButtonGrid.h"
#include "S4Button.h"
#include "Seq4.h"
#include "UndoRedoStack.h"
#include "../Sequencer4Widget.h"

#ifdef _SEQ4

S4Button* S4ButtonGrid::getButton(int row, int col) {
    assert(row >= 0 && row < 4 && col >= 0 && col < 4);
    return buttons[row][col];
}

void S4ButtonGrid::setNewSeq(MidiSequencer4Ptr newSeq) {
    seq = newSeq;
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        for (int col = 0; col < MidiSong4::numTracks; ++col) {
            buttons[row][col]->setNewSeq(newSeq);
        }
    }
}


/***************************** S4ButtonGrid ***********************************/

using Comp = Seq4<WidgetComposite>;
void S4ButtonGrid::init(Sequencer4Widget* parent, rack::engine::Module* module,
                        MidiSequencer4Ptr _seq, std::shared_ptr<Seq4<WidgetComposite>> _seq4Comp) {
    widget = parent;
    seq = _seq;

    MidiSong4Ptr song;
    if (seq) {
        song = seq->song;
    } else {
        song =  MidiSong4::makeTest(MidiTrack::TestContent::eightQNotesCMaj, 0, 0);
    }

    if (!seq) {
       seq = MidiSequencer4::make(song);
    }

    if (!seq->song) {
        seq->song = MidiSong4::makeTest(MidiTrack::TestContent::eightQNotesCMaj, 0, 0);
    }
 
   // std::shared_ptr<IComposite> icomp = Comp::getDescription();

    seq4Comp = _seq4Comp;
    const float jacksX = 380;
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        const float y = 70 + row * (buttonSize + buttonMargin);
        for (int col = 0; col < MidiSong4::numSectionsPerTrack; ++col) {
            const float x = 130 + col * (buttonSize + buttonMargin);
            const int padNumber = row * MidiSong4::numSectionsPerTrack + col;


            S4Button* button = new S4Button(
                rack::math::Vec(buttonSize, buttonSize),
                rack::math::Vec(x, y),
                row,
                col,
                seq,
                seq4Comp,
                module);

#if 1   // param widget way
            if (module) {
                button->paramQuantity = module->paramQuantities[Comp::PADSELECT0_PARAM + padNumber];
                 if (padNumber==0) {
              //  INFO("added pq=%p, from param=%d pad=%d", button->paramQuantity, Comp::PADSELECT0_PARAM + padNumber, padNumber);
              //  INFO("min=%f max = %f param=%p", button->paramQuantity->minValue, button->paramQuantity->maxValue,                    button->paramQuantity->getParam());
                 }
                    
            }
             if (padNumber==0) {
            INFO("making button x=%.2f, box.pos.x=%.2f ", x, button->box.pos.x);
            INFO("button type=%s parent=%s, %p", typeid(button).name(), typeid(parent).name(), parent);
             }
            parent->addParam(button);
#else
           
            parent->addChild(b);
#endif
            
            button->setClickHandler(makeButtonHandler(row, col));
            buttons[row][col] = button;
        }

        const float jacksY = y + 8;
        const float jacksDy = 28;

        parent->addOutput(rack::createOutputCentered<rack::componentlibrary::PJ301MPort>(
            rack::math::Vec(jacksX, jacksY),
            module,
            Comp::CV0_OUTPUT + row));
        parent->addOutput(rack::createOutputCentered<rack::componentlibrary::PJ301MPort>(
            rack::math::Vec(jacksX, jacksY + jacksDy),
            module,
            Comp::GATE0_OUTPUT + row));

        parent->addInput(rack::createInputCentered<rack::componentlibrary::PJ301MPort>(
            rack::math::Vec(30, jacksY + 1 + jacksDy / 2),
            module,
            Comp::MOD0_INPUT + row));
    }
}

class S4ButtonClickCommand : public Sq4Command
{
public:
    S4ButtonClickCommand(int row, int col) : rowToSelect(row), colToSelect(col) 
    {

    }

    void execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget) override
    {
        // we always need to get a fresh pointer - can't store in undo object
        assert(widget);
        std::shared_ptr<S4ButtonGrid> grid = widget->getButtonGrid();
        assert(grid);

       // select the one we just clicked into
        for (int r = 0; r < MidiSong4::numTracks; ++r) {
            for (int c = 0; c < MidiSong4::numSectionsPerTrack; ++c) {
                auto button = grid->getButton(r, c);
                assert(button);
                if (button->isSelected()) {
                    origRowSelected = r;
                    origColSelected = c;
                }
                button->setSelection(r == rowToSelect && c == colToSelect);
            }
        }
 
        auto button = grid->getButton(rowToSelect, colToSelect);
        button->doEditClip();
    }
    
    void undo(MidiSequencer4Ptr seq, Sequencer4Widget* widget) override
    {
        // we always need to get a fresh pointer - can't store in undo object
        assert(widget);
        std::shared_ptr<S4ButtonGrid> grid = widget->getButtonGrid();
        assert(grid);

        INFO("S4ButtonClickCommand::undo ==== %p", this);
        // first restore the edited track
        if (origColSelected >= 0 && origRowSelected >= 0) {
            auto button = grid->getButton(origRowSelected, origColSelected);
            assert(button);
            INFO("S4ButtonClickCommand::undo setting edit to %d,%d",origRowSelected, origColSelected);
            button->doEditClip();
        }

        // unset the selection we set before
        auto button = grid->getButton(rowToSelect, colToSelect);
        button->setSelection(false);
        INFO("S4ButtonClickCommand::undo un-setting edit from %d,%d", rowToSelect, colToSelect);

        // and select the original
        if (origRowSelected >= 0 && origColSelected >= 0) {
            button = grid->getButton(origRowSelected, origColSelected);
            button->setSelection(true);
            INFO("S4ButtonClickCommand::undo setting sel to %d,%d", origRowSelected, origColSelected);
        }
        INFO("S4ButtonClickCommand::undo (exit) ==== %p", this);

    }

private:
    const int rowToSelect;
    const int colToSelect;
    int origRowSelected = -1;
    int origColSelected = -1;
};

void S4ButtonGrid::onClick(bool isCtrl, int row, int col) {
    Command4Ptr cmd = std::make_shared<S4ButtonClickCommand>(row, col);
    cmd->name = "click";
    assert(widget);
    seq->undo->execute4(seq, widget, cmd);
#if 0
    // select the one we just clicked into
    for (int r = 0; r < MidiSong4::numTracks; ++r) {
        for (int c = 0; c < MidiSong4::numSectionsPerTrack; ++c) {
            auto button = getButton(r, c);
            assert(button);
            button->setSelection(r == row && c == col);
        }
    }
#endif

    // we still do our own selection of next section, outside of unto
    if (isCtrl) {
        // then the select next clip
        // remember, section is 1..4
        seq4Comp->setNextSectionRequest(row, col + 1);
    } else {
        for (int r = 0; r < MidiSong4::numTracks; ++r) {
            seq4Comp->setNextSectionRequest(r, col + 1);
        }
    }
  //  auto button = getButton(row, col);

  //  button->doEditClip();
}

std::function<void(bool isCtrlKey)> S4ButtonGrid::makeButtonHandler(int row, int col) {
    return [this, row, col](bool isCtrl) {
        this->onClick(isCtrl, row, col);
    };
}

#endif
