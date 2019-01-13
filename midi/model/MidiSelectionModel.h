#pragma once
#include <memory>
#include <set>

class MidiEvent;

/**
 * Central manager for tracking selections in the MidiSong being edited.
 */
class MidiSelectionModel
{
public:
    MidiSelectionModel();
    ~MidiSelectionModel();
    /** 
     * replace the current selection with a single event
     */
    void select(std::shared_ptr<MidiEvent>);

    using container = std::set<std::shared_ptr<MidiEvent>>;
    using const_iterator = container::const_iterator;
   // using iterator_pair = std::pair<const_iterator, const_iterator>;

 
    const_iterator begin() const;
    const_iterator end() const;

    int size() const
    {
        return (int) selection.size();
    }
    bool empty() const
    {
        return selection.empty();
    }

    bool isSelected(std::shared_ptr<MidiEvent>) const;
private:

    
    container selection;
};

using MidiSelectionModelPtr = std::shared_ptr<MidiSelectionModel>;