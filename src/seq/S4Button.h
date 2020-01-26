#pragma once

//#include "Widget.hpp"
#include "rack.hpp"
#include "math.hpp"
#include "SqClipboard.h"
#include "SqGfx.h"
#include "TimeUtils.h"
#include "UIPrefs.h"
#include "MidiSequencer4.h"

#include <functional>

class S4Button;
class MidiTrack;
class MidiTrack4Options;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiTrack4OptionsPtr = std::shared_ptr<MidiTrack4Options>;

class S4ButtonDrawer : public ::rack::OpaqueWidget
{
public:
    S4ButtonDrawer(const rack::math::Vec& size, const rack::math::Vec& pos, S4Button* button) :
        button(button)
    {
        this->box.size=size;
    }
    void draw(const DrawArgs &args) override;
private:
    S4Button* const button;

};

class S4Button : public ::rack::OpaqueWidget
{
public:
    friend class S4ButtonDrawer;
    friend class RepeatItem;
    S4Button(const rack::math::Vec& size, const rack::math::Vec& pos, int r, int c, MidiSong4Ptr s);

    /**
     * pass callback here to handle clicking on LED
     */
    using callback = std::function<void(bool isCtrlKey)>;
    void setClickHandler(callback);
    void setSelection(bool);

    void onButton(const rack::event::Button &e) override;
    void onDragHover(const rack::event::DragHover &e) override;
    void onDragEnter(const rack::event::DragEnter &e) override;
    void onDragLeave(const rack::event::DragLeave &e) override;
    void onSelectKey(const rack::event::SelectKey &e) override;

    bool isSelected() const
    {
        return _isSelected;
    }

    void step() override;

    void setNewSeq(MidiSequencer4Ptr newSeq)
    {
        song = newSeq->song;
    }

private:
    rack::widget::FramebufferWidget * fw = nullptr;
    S4ButtonDrawer * drawer = nullptr;
    callback clickHandler = nullptr;
    bool isDragging = false;
  
    const int row;
    const int col;
    MidiSong4Ptr song;
    bool _isSelected = false;
    std::string contentLength;
    int numNotes = 0;

    bool handleKey(int key, int mods, int action);
    void doPaste();
    MidiTrackPtr getTrack() const;
    MidiTrack4OptionsPtr getOptions() const;
    void invokeContextMenu();

    int getRepeatCountForUI();
};

/**
 * A special purpose button for the 4x4 seq module.
 * Has simple click handling, but lots of dedicated drawing ability
 */
inline void S4ButtonDrawer::draw(const DrawArgs &args)
{
    auto ctx = args.vg;
    if (button->isSelected()) {
          SqGfx::filledRect(
                args.vg,
                UIPrefs::X4_SELECTION_COLOR,
                this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
    } else {
        SqGfx::filledRect(
                args.vg,
                UIPrefs::NOTE_COLOR,
                this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
                //x, y, width, noteHeight);
    }

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14.f);
    nvgFillColor(ctx, UIPrefs::TIME_LABEL_COLOR);
    nvgText(ctx, 5, 15, button->contentLength.c_str(), nullptr);
    if (button->numNotes > 0) {
        std::stringstream s;
        s << button->numNotes;
        nvgText(ctx, 5, 30, s.str().c_str(), nullptr);
    }
}

inline S4Button::S4Button(
    const rack::math::Vec& size, 
    const rack::math::Vec& pos,
    int r, 
    int c, 
    MidiSong4Ptr s) : row(r), col(c), song(s)
{
    this->box.size = size;
    this->box.pos = pos;
    fw = new rack::widget::FramebufferWidget();
    this->addChild(fw);

    drawer = new S4ButtonDrawer(size, pos, this);
    fw->addChild(drawer);
}

inline void S4Button::setSelection(bool sel)
{
    if (_isSelected != sel) {
        _isSelected = sel;
        fw->dirty = true;
    }
}

inline bool S4Button::handleKey(int key, int mods, int action)
{
    bool handled = false;
    
    if ((key == GLFW_KEY_V) && 
        (!(mods & RACK_MOD_CTRL)) &&
        (action == GLFW_PRESS)) {

        handled = true;
        doPaste();
    }
    return handled;
}

inline void S4Button::onSelectKey(const rack::event::SelectKey &e)
{
    bool handled = handleKey(e.key, e.mods, e.action);
    if (handled) {
        e.consume(this);
    } else {
        OpaqueWidget::onSelectKey(e);
    }
}

inline void S4Button::setClickHandler(callback h)
{
    clickHandler = h;
}

inline void S4Button::onDragEnter(const rack::event::DragEnter &e)
{
}

inline void S4Button::onDragLeave(const rack::event::DragLeave &e) 
{
    isDragging = false;
}

/***************************************************************************
 * 
 * S4ButtonGrid
 * 
 * 
 * bridge between the widget and the buttons
 * 
 ****************************************************************************/

#include "MidiSong4.h"

class S4ButtonGrid
{
public:
    void init(rack::app::ModuleWidget* widget, rack::engine::Module* module, MidiSong4Ptr s);
    void setNewSeq(MidiSequencer4Ptr newSeq);
    const static int buttonSize = 50.f;
    const static int buttonMargin = 10;
private:
    std::function<void(bool isCtrlKey)> makeButtonHandler(int row, int column);
    S4Button* getButton(int row, int col);
    S4Button* buttons[MidiSong4::numTracks][MidiSong4::numSectionsPerTrack] = {{}};
};

inline S4Button* S4ButtonGrid::getButton(int row, int col)
{
    assert(row>=0 && row<4 && col>=0 && col<4);
    return buttons[row][col];
}

inline void S4ButtonGrid::setNewSeq(MidiSequencer4Ptr newSeq)
{
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        for (int col = 0; col < MidiSong4::numTracks; ++col) {   
        
            buttons[row][col]->setNewSeq(newSeq);
        }
    }

}

inline std::function<void(bool isCtrlKey)> S4ButtonGrid::makeButtonHandler(int row, int col)
{
    return [row, col, this](bool isCtrl) {
        for (int r = 0; r < MidiSong4::numTracks; ++r) {
            for (int c = 0; c < MidiSong4::numTracks; ++c) {
                auto button = getButton(r, c);
                assert(button);
                button->setSelection(r==row && c==col);
            }
        }  
    };
}


