#pragma once
#include <memory>
#include <set>

class MidiEvent;

class MidiSelectionModel
{
public:
    /** 
     * replace the current selection with a single event
     */
    void select(std::shared_ptr<MidiEvent>);

    using container = std::set<std::shared_ptr<MidiEvent>>;
    using const_iterator = container::const_iterator;
    using iterator_pair = std::pair<const_iterator, const_iterator>;

    iterator_pair get() const;
private:

    
    container selection;
};

using MidiSelectionPtr = std::shared_ptr<MidiSelectionModel>;