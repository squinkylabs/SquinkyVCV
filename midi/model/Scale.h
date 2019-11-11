#pragma once

#include <map>
#include <memory>
#include <vector>

class Scale;
class ScaleRelativeNote;
using ScalePtr = std::shared_ptr<Scale>;
using ScaleRelativeNotePtr = std::shared_ptr<ScaleRelativeNote>;

class Scale : public std::enable_shared_from_this<Scale>
{
public:
    enum class Scales {
        Major,
        Dorian,
        Phrygian,
        Lydian,
        Mixolydian,
        Minor,
        Locrian
    };

    /**
     *  Factory method
     */
    static ScalePtr getScale(Scales, int);

    std::shared_ptr<Scale> getptr() {
        return shared_from_this();
    }

   
    // semitones are absolute semis, as used in PitchUtils
    ScaleRelativeNotePtr getScaleRelativeNote(int semitone);
    int getSemitone(const ScaleRelativeNote&);

    /**
     * Input and output are regular chromatic semitones,
     * But transpose will be done scale relative
     */
    int transposeInScale(int semitone, int scaleDegreesToTranspose);
private:
    /**
     * To create a Scale, first you must new one,
     * then call init on it.
     * It's all a contructor / shared_from_this issue
     */
    Scale();
    void init(Scales scale, int keyRoot);
    
    int degreesInScale() const;

    /**
     *  make from semi-normalized semitones to srn
     * example: 0 is C. 11 is B. so B major would have 11, 13 ....
     */
    std::map<int, ScaleRelativeNotePtr> abs2srn;

    static std::vector<int> getBasePitches(Scales);
};