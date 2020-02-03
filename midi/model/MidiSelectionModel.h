#pragma once
#include <memory>
#include <set>
#include <vector>

class MidiEvent;
class MidiTrack;
class MidiSelectionModel;
class IMidiPlayerAuditionHost;

using MidiSelectionModelPtr = std::shared_ptr<MidiSelectionModel>;
using IMidiPlayerAuditionHostPtr = std::shared_ptr<IMidiPlayerAuditionHost>;
using MidiEventPtr = std::shared_ptr<MidiEvent>;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;

/**
 * Central manager for tracking selections in the MidiSong being edited.
 */
class MidiSelectionModel
{
public:
    MidiSelectionModel(IMidiPlayerAuditionHostPtr);
    MidiSelectionModel(IMidiPlayerAuditionHostPtr, bool selectAll);
    ~MidiSelectionModel();

    /**
     * replace the current selection with a single event
     */
    void select(MidiEventPtr);
    void extendSelection(MidiEventPtr);
    void addToSelection(MidiEventPtr, bool keepExisting);
    void removeFromSelection(MidiEventPtr);

    void selectAll(MidiTrackPtr);
    bool isAllSelected() const;

    bool isAuditionSuppressed() const;
    void setAuditionSuppressed(bool);

    /**
     * select nothing
     */
    void clear();

    class CompareEventPtrs
    {
    public:
        bool operator() (const MidiEventPtr& lhs, const MidiEventPtr& rhs) const;
    };

    using container = std::set<MidiEventPtr, CompareEventPtrs>;
    using const_iterator = container::const_iterator;
    using const_reverse_iterator = container::const_reverse_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    int size() const
    {
        return (int) selection.size();
    }
    bool empty() const
    {
        return selection.empty();
    }

    MidiSelectionModelPtr clone() const;

    /**
     * retrieve all the events in order into a vector
     */
    std::vector<MidiEventPtr> asVector() const;

    MidiEventPtr getLast();

    /** Returns true is this object instance is in selection.
     * i.e. changes on pointer value.
     * O(1)
     */
    bool isSelected(MidiEventPtr) const;

    /** Returns true is there is an object in selection equivalent
     * to 'event'. i.e.  selection contains entry == *event.
     * O(n), where n is the number of items in selection
     */
    bool isSelectedDeep(MidiEventPtr event) const;

    IMidiPlayerAuditionHostPtr _testGetAudition();

private:

    void add(MidiEventPtr);

    container selection;

    IMidiPlayerAuditionHostPtr auditionHost;
    bool auditionSuppressed = false;
    bool allIsSelected = false;
};
