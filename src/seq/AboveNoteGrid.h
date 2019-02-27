#pragma once

#include "../Squinky.hpp"
#ifdef __V1
#include "widget/Widget.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#endif

class MidiSequencer;
using  MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

struct AboveNoteGrid : OpaqueWidget
{
public:
    AboveNoteGrid(const Vec& pos, const Vec& size, MidiSequencerPtr seq);

    /**
     * Inject a new sequencer into this editor.
     */
    void setSequencer(MidiSequencerPtr seq);

#ifdef __V1
	//void onSelect(const event::Select &e) override;
	//void onDeselect(const event::Deselect &e) override;
   // void onSelectKey(const event::SelectKey &e) override;
    void draw(const DrawArgs &args) override;
#else
    void draw(NVGcontext *vg) override;
    //void onFocus(EventFocus &e) override;
   // void onDefocus(EventDefocus &e) override;
   //void onKey(EventKey &e) override;
#endif

private:
    MidiSequencerPtr sequencer;

};