
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
    NoteDisplay(const Vec& pos, const Vec& size, MidiSequencerPtr seq);

    // These drawing utils could go elsewhere
    static void strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    static void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h);
    
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

    // mouse stuff (move some to manager)
  
    std::shared_ptr<class MouseManager> mouseManager;
  //  Vec lastMouseClickPos;
   // std::shared_ptr<class NoteDragger> noteDragger;

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
   
#ifdef __V1
    void onSelect(const SelectEvent &e) override;
    void onDeselect(const DeselectEvent &e) override;
    void draw(const DrawArgs &args) override;
	void onDoubleClick(const widget::DoubleClickEvent &e) override;
    void onButton(const ButtonEvent &e) override;
    void onHoverKey(const HoverKeyEvent &e) override;
    void onSelectKey(const SelectKeyEvent &e) override;
    void onDragStart(const DragStartEvent &e) override;
	void onDragEnd(const DragEndEvent &e) override;
	void onDragMove(const DragMoveEvent &e)  override;
    void onDragDrop(const DragDropEvent &e) override;
    bool handleKey(int key, int mods, int action);
#else
    void draw(NVGcontext *vg) override;
    void onFocus(EventFocus &e) override;
    void onDefocus(EventDefocus &e) override;
    void onKey(EventKey &e) override;
    //void onHoverKey(EventHoverKey &e) override;
#endif

};
