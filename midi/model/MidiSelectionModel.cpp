
#include "IMidiPlayerHost.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"

#include <assert.h>
extern int _mdb;
MidiSelectionModel::MidiSelectionModel(IMidiPlayerAuditionHostPtr aud) : auditionHost(aud)
{
    ++_mdb;
}

MidiSelectionModel::~MidiSelectionModel()
{
    --_mdb;
}


bool MidiSelectionModel::CompareEventPtrs::operator() (const std::shared_ptr<MidiEvent>& lhs, const std::shared_ptr<MidiEvent>& rhs) const

{
    MidiEvent& le = *lhs;
    MidiEvent& re = *rhs;
    return  le < re;
}

void MidiSelectionModel::select(std::shared_ptr<MidiEvent> event)
{
    selection.clear();
    assert(selection.empty());
    add(event);
}
void MidiSelectionModel::extendSelection(std::shared_ptr<MidiEvent> event)
{
    add(event);
}

void MidiSelectionModel::addToSelection(std::shared_ptr<MidiEvent> event, bool keepExisting)
{
    auto it = selection.find(event);
    if (it != selection.end()) {
        // if note is already in, then don't clear and re-add
        return;
    }

    if (!keepExisting) {
        selection.clear();
    }
    add(event);
}

void MidiSelectionModel::removeFromSelection(std::shared_ptr<MidiEvent> event)
{
    auto it = selection.find(event);
    assert(it != selection.end());
    if (it != selection.end()) {
        selection.erase(it);
    }
}

MidiSelectionModel::const_iterator MidiSelectionModel::begin() const
{
    return selection.begin();
}

MidiSelectionModel::const_iterator MidiSelectionModel::end() const
{
    return selection.end();
}

void MidiSelectionModel::clear()
{
    selection.clear();
}

void MidiSelectionModel::add(MidiEventPtr evt)
{
    auto found = selection.find(evt);
    if (found != selection.end()) {
        // if the event is already there, don't do anything.
        return;
    }
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(evt);
    if (note) {
        auditionHost->auditionNote(note->pitchCV);
    }
    selection.insert(evt);
}

bool MidiSelectionModel::isSelected(MidiEventPtr evt) const
{
    assert(evt);
    auto it = std::find(selection.begin(), selection.end(), evt);
    return it != selection.end();
}

MidiEventPtr MidiSelectionModel::getLast()
{
    MidiEventPtr ret;
    float lastTime = 0;
    for (auto it : selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            float noteEnd = note->startTime + note->duration;
            if (noteEnd > lastTime) {
                ret = note;
                lastTime = noteEnd;
            }
        } else {
            float end = it->startTime;
            if (end > lastTime) {
                ret = it;
                lastTime = end;
            }
        }
    }
    return ret;
}

MidiSelectionModelPtr MidiSelectionModel::clone() const
{
    MidiSelectionModelPtr ret = std::make_shared<MidiSelectionModel>(auditionHost);
    for (auto it : selection) {
        MidiEventPtr clonedEvent = it->clone();
        ret->add(clonedEvent);
    }
    return ret;
}

bool MidiSelectionModel::isSelectedDeep(MidiEventPtr evt) const
{
    auto it = std::find_if(begin(), end(), [evt](MidiEventPtr ev) {
        return *ev == *evt;
    });

    return it != end();
}
