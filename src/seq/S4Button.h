#pragma once

#include "MidiSequencer4.h"
#include "Seq4.h"
#include "TimeUtils.h"
#include "WidgetComposite.h"
#include "math.hpp"
#include "rack.hpp"

#include <functional>

class S4Button;
class MidiTrack;
class MidiTrack4Options;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using MidiTrack4OptionsPtr = std::shared_ptr<MidiTrack4Options>;

class S4ButtonDrawer : public ::rack::OpaqueWidget  {
public:
    S4ButtonDrawer(const rack::math::Vec& size, S4Button* button);
    void draw(const DrawArgs& args) override;

    void onEnter(const rack::event::Enter& e) override;
	void onLeave(const rack::event::Leave& e) override;

private:
    void paintButtonFace(NVGcontext*);
    void paintButtonBorder(NVGcontext*);
    void paintButtonText(NVGcontext*);

    S4Button* const button;
};

class S4Button : public ::rack::app::ParamWidget {
public:
    friend class S4ButtonDrawer;
    friend class RepeatItem;
    friend class EditMenuItems;
    S4Button(const rack::math::Vec& size,
             const rack::math::Vec& pos,
             int r, int c,
             MidiSequencer4Ptr s,
             std::shared_ptr<Seq4<WidgetComposite>> seq4Comp,
             ::rack::engine::Module* theModule);

    /**
     * pass callback here to handle clicking on LED
     */
    using callback = std::function<void(bool isCtrlKey)>;
    void setClickHandler(callback);
    void setSelection(bool);

    void onDragHover(const rack::event::DragHover& e) override;
    void onButton(const rack::event::Button& e) override;
    void onDragStart(const rack::event::DragStart& e) override;
    void onSelectKey(const rack::event::SelectKey& e) override;
    bool isSelected() const {
        return _isSelected;
    }

    void step() override;

    void setNewSeq(MidiSequencer4Ptr newSeq) {
        seq = newSeq;
    }

    void doEditClip();

private:
    rack::widget::FramebufferWidget* fw = nullptr;
   
    callback clickHandler = nullptr;
    bool isDragging = false;

    const int row;
    const int col;
    MidiSequencer4Ptr seq;
    std::shared_ptr<Seq4<WidgetComposite>> seq4Comp;

    /**
     * state variables that affect drawing
     */
    bool _isSelected = false;
    std::string contentLength;
    int numNotes = 0;
    bool isPlaying = false;
    bool iAmNext = false;
    int repeatCount = 1;
    int repetitionNumber = 1;
    ::rack::engine::Module* const module;
    const int selectParamId;
    bool lastSelectParamState = false;
    bool mouseButtonIsControlKey = false;;

    bool handleKey(int key, int mods, int action);
    void doCut();
    void doCopy();
    void doPaste();
    MidiTrackPtr getTrack() const;
    MidiTrack4OptionsPtr getOptions() const;
    void invokeContextMenu();

    int getRepeatCountForUI();
    void setRepeatCountForUI(int);
    void otherItems(::rack::ui::Menu* menu);
    void pollForParamChange();
};




