
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
    selection.insert(event);
}

MidiSelectionModel::const_iterator MidiSelectionModel::begin() const
{
    return selection.begin();
}

MidiSelectionModel::const_iterator MidiSelectionModel::end() const
{
    return selection.end();
}
#if 0
MidiSelectionModel::iterator_pair MidiSelectionModel::get() const
{
    return iterator_pair(selection.begin(), selection.end());
}
#endif