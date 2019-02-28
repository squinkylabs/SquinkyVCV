
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
    Label* focusLabel = nullptr;
  //  Label* editAttributeLabel = nullptr;
  //  Label* barRangeLabel = nullptr;
  //  std::shared_ptr<NoteScreenScale> scaler;
    MidiSequencerPtr sequencer;
    bool cursorState = false;
    int cursorFrameCount = 0;
    bool haveFocus = true;
    void initEditContext();

public:
    NoteDisplay(const Vec& pos, const Vec& size, MidiSequencerPtr seq);

    /**
     * Inject a new sequencer into this editor.
     */
    void setSequencer(MidiSequencerPtr seq);

    void step() override;

    void updateFocus(bool focus)
    {
        if (focus != haveFocus) {
            haveFocus = focus;
            focusLabel->text = focus ? "" : "Click in editor to get focus";
        }
    }

    void drawNotes(NVGcontext *vg);
    void drawCursor(NVGcontext *vg);
    void drawGrid(NVGcontext *vg);
    void drawBackground(NVGcontext *vg);
    void strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);

#ifdef __V1
    void onSelect(const event::Select &e) override;
    void onDeselect(const event::Deselect &e) override;
    void onSelectKey(const event::SelectKey &e) override;
    void draw(const DrawArgs &args) override;
#else
    void draw(NVGcontext *vg) override;
    void onFocus(EventFocus &e) override;
    void onDefocus(EventDefocus &e) override;
    void onKey(EventKey &e) override;
#endif

};
