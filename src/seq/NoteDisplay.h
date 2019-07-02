
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
public:
    NoteDisplay(
        const Vec& pos,
        const Vec& size,
        MidiSequencerPtr seq,
        rack::engine::Module* mod);


    /**
     * Inject a new sequencer into this editor.
     */
    void setSequencer(MidiSequencerPtr seq);
    MidiSequencerPtr getSequencer();
private:
    Label* focusLabel = nullptr;
    MidiSequencerPtr sequencer;
    bool cursorState = false;
    int cursorFrameCount = 0;
    bool haveFocus = true;
    void initEditContext();

    std::shared_ptr<class MouseManager> mouseManager;

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

    static bool isKeyWeNeedToStealFromRack(int key);
#ifdef __V1x
    void onSelect(const event::Select &e) override;
    void onDeselect(const event::Deselect &e) override;
    void draw(const DrawArgs &args) override;
    void onDoubleClick(const event::DoubleClick &e) override;
    void onButton(const event::Button &e) override;
    void onHoverKey(const event::HoverKey &e) override;
    void onSelectKey(const event::SelectKey &e) override;
    void onDragStart(const event::DragStart &e) override;
    void onDragEnd(const event::DragEnd &e) override;
    void onDragMove(const event::DragMove &e)  override;
    void onDragDrop(const event::DragDrop &e) override;
    bool handleKey(int key, int mods, int action);
#else
    void draw(NVGcontext *vg) override;
    void onFocus(EventFocus &e) override;
    void onDefocus(EventDefocus &e) override;
    void onKey(EventKey &e) override;
    //void onHoverKey(EventHoverKey &e) override;
#endif
};
