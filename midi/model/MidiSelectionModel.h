#pragma once
#include <memory>
#include <set>

class MidiEvent;
class MidiSelectionModel;
class IMidiPlayerAuditionHost;

using MidiSelectionModelPtr = std::shared_ptr<MidiSelectionModel>;
using IMidiPlayerAuditionHostPtr = std::shared_ptr<IMidiPlayerAuditionHost>;

/**
 * Central manager for tracking selections in the MidiSong being edited.
 */
class MidiSelectionModel
{
public:
    MidiSelectionModel(IMidiPlayerAuditionHostPtr);
    ~MidiSelectionModel();
    /**
     * replace the current selection with a single event
     */
    void select(std::shared_ptr<MidiEvent>);
    void extendSelection(std::shared_ptr<MidiEvent>);
    void addToSelection(std::shared_ptr<MidiEvent>, bool keepExisting);
    void removeFromSelection(std::shared_ptr<MidiEvent>);

    bool isAuditionSuppressed() const;
    void setAuditionSuppressed(bool);

    /**
     * select nothing
     */
    void clear();

    class CompareEventPtrs
    {
    public:
        bool operator() (const std::shared_ptr<MidiEvent>& lhs, const std::shared_ptr<MidiEvent>& rhs) const;
    };

    using container = std::set<std::shared_ptr<MidiEvent>, CompareEventPtrs>;
    using const_iterator = container::const_iterator;

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

    MidiSelectionModelPtr clone() const;

    std::shared_ptr<MidiEvent> getLast();

    /** Returns true is this object instance is in selection.
     * i.e. changes on pointer value.
     * O(1)
     */
    bool isSelected(std::shared_ptr<MidiEvent>) const;

    /** Returns true is there is an object in selection equivalent
     * to 'event'. i.e.  selection contains entry == *event.
     * O(n), where n is the number of items in selection
     */
    bool isSelectedDeep(std::shared_ptr<MidiEvent> event) const;

    IMidiPlayerAuditionHostPtr _testGetAudition();

private:

    void add(std::shared_ptr<MidiEvent>);

    container selection;

    IMidiPlayerAuditionHostPtr auditionHost;
    bool auditionSuppressed = false;
};
