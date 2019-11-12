#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

class Scale;
class MidiEvent;
class ScaleRelativeNote;
using ScalePtr = std::shared_ptr<Scale>;
using ScaleRelativeNotePtr = std::shared_ptr<ScaleRelativeNote>;
using MidiEventPtr = std::shared_ptr<MidiEvent>;
 using XformLambda = std::function<void(MidiEventPtr)>;
 

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

   
    static XformLambda makeTransposeLambdaChromatic(int transposeSemitones);
    static XformLambda makeTransposeLambdaScale(int scaleDegrees, int keyRoot, Scales mode);

  // static std::function<void(MidiEventPtr)> makeInvertLambda(int invertAxisSemitones, bool constrainToKeysig, int keyRoot, Modes mode);
   // static std::function<void(MidiEventPtr)> makeInvertLambdaChromatic(int invertAxisSemitones);
   // static std::function<void(MidiEventPtr)> makeInvertLambdaDiatonic(int invertAxisSemitones, int keyRoot, DiatonicUtils::Modes mode);

    int degreesInScale() const;

    /**
     * returns octave:degree from degree.
     */
    std::pair<int, int> normalizeDegree(int);
private:
    /**
     * To create a Scale, first you must new one,
     * then call init on it.
     * It's all a contructor / shared_from_this issue
     */
    Scale();
    void init(Scales scale, int keyRoot);
    
    
    int transposeInScaleChromatic(int semitone, int scaleDegreesToTranspose);

    /**
     *  make from semi-normalized semitones to srn
     * example: 0 is C. 11 is B. so B major would have 11, 13 ....
     */
    std::map<int, ScaleRelativeNotePtr> abs2srn;

    static std::vector<int> getBasePitches(Scales);
};