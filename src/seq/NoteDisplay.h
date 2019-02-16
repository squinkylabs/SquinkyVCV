
#pragma once

#include "MidiSequencer.h"
#include "NoteScreenScale.h"

/**
 * This class needs some refactoring and renaming.
 * It is really the entire sequencer UI, including the notes.
 * 
 * Pretty soon we should sepparate out the NoteEditor.
 */
struct NoteDisplay : OpaqueWidget
{
private:
    Label* focusLabel=nullptr;
    Label* editAttributeLabel = nullptr;
    Label* barRangeLabel = nullptr;
    std::shared_ptr<NoteScreenScale> scaler;
    MidiSequencerPtr sequencer;
    bool cursorState = false;
    int cursorFrameCount = 0;
    bool haveFocus = true;
    MidiEditorContext::NoteAttribute curAttribute = MidiEditorContext::NoteAttribute::Duration;
    int curFirstBar = -1;

public:
    NoteDisplay(const Vec& pos, const Vec& size, MidiSequencerPtr seq);

    void step() override;

    void updateFocus(bool focus) {
        if (focus != haveFocus) {
            haveFocus = focus;
            focusLabel->text = focus ? "" : "Click in editor to get focus";
        }
    }

    void drawNotes(NVGcontext *vg);
    void drawCursor(NVGcontext *vg) ;
    void draw(NVGcontext *vg) override;
    void drawBackground(NVGcontext *vg);
    void strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    void onFocus(EventFocus &e) override;
    void onDefocus(EventDefocus &e) override;
    void onKey(EventKey &e) override;

    //************************** These overrides are just to test even handling
    void onMouseDown(EventMouseDown &e) override
    {
        OpaqueWidget::onMouseDown(e);
      //  std::cout << "onMouseDown " << e.button << std::flush << std::endl;
    }
    void onMouseMove(EventMouseMove &e) override
    {
       //  std::cout << "onMouseMove " << std::flush << std::endl;
        OpaqueWidget::onMouseMove(e);
    }

    void onMouseEnter(EventMouseEnter &e) override
    {
        //std::cout << "nmouseenger " << std::flush << std::endl;
    }
   /** Called when another widget begins responding to `onMouseMove` events */
//	virtual void onMouseLeave(EventMouseLeave &e) {}

};
