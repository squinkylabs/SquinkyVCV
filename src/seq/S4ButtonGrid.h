
#pragma once

/***************************************************************************
 * 
 * S4ButtonGrid
 * 
 * 
 * bridge between the widget and the buttons
 * 
 ****************************************************************************/

#include "MidiSong4.h"

#include "Seq4.h"
//#include "TimeUtils.h"
#include "WidgetComposite.h"
#include "rack.hpp"

class S4Button;

class S4ButtonGrid {
public:
    void init(
        rack::app::ModuleWidget* widget,
        rack::engine::Module* module,
        MidiSequencer4Ptr seq,
        std::shared_ptr<Seq4<WidgetComposite>> seq4Comp);
    void setNewSeq(MidiSequencer4Ptr newSeq);
    const static int buttonSize = 50.f;
    const static int buttonMargin = 10;

private:
    std::function<void(bool isCtrlKey)> makeButtonHandler(int row, int column);
    S4Button* getButton(int row, int col);
    S4Button* buttons[MidiSong4::numTracks][MidiSong4::numSectionsPerTrack] = {{}};
    void onClick(bool isCtrlKey, int row, int col);
    // void onEditClip();

    std::shared_ptr<Seq4<WidgetComposite>> seq4Comp;
};
