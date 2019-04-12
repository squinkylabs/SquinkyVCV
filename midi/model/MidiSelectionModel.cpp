
#include "MidiEvent.h"
#include "MidiSelectionModel.h"

#include <assert.h>
extern int _mdb;
MidiSelectionModel::MidiSelectionModel()
{
    ++_mdb;
}

MidiSelectionModel::~MidiSelectionModel()
{
    --_mdb;
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

//void addToSelection(std::shared_ptr<MidiEvent>, bool keepExisting);

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
   // selection.push_back(evt);
    selection.insert(evt);
}

bool MidiSelectionModel::isSelected(MidiEventPtr evt) const
{
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
    MidiSelectionModelPtr ret = std::make_shared<MidiSelectionModel>();
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
