
#include "MidiSelectionModel.h"

#include <assert.h>

void MidiSelectionModel::select(std::shared_ptr<MidiEvent> event)
{
    selection.clear();
    assert(selection.empty());
    selection.insert(event);
}

MidiSelectionModel::iterator_pair MidiSelectionModel::get() const
{
    return iterator_pair(selection.begin(), selection.end());
}