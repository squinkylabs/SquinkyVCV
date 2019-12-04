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
 

class Scale
{
public:
    enum class Scales {
        Major,
        Dorian,
        Phrygian,
        Lydian,
        Mixolydian,
        Minor,
        Locrian,
        MinorPentatonic,
        HarmonicMinor,
        Diminished,
        DominantDiminished,
        WholeStep
    };

    /**
     *  Factory method
     */
    static ScalePtr getScale(Scales, int);

    // semitones are absolute semis, as used in PitchUtils
    ScaleRelativeNote getScaleRelativeNote(int semitone) const;
    ScaleRelativeNotePtr getScaleRelativeNotePtr(int semitone) const;
    int getSemitone(const ScaleRelativeNote&) const;

    /**
     * Input and output are regular chromatic semitones,
     * But transpose will be done scale relative
     */
    int transposeInScale(int semitone, int scaleDegreesToTranspose) const;

    int invertInScale(int semitone, int inversionDegree) const;

    int quantizeToScale(int semitone) const;

    static XformLambda makeTransposeLambdaChromatic(int transposeSemitones);
    static XformLambda makeTransposeLambdaScale(int scaleDegrees, int keyRoot, Scales mode);

    static XformLambda makeInvertLambdaChromatic(int invertAxisSemitones);
    static XformLambda makeInvertLambdaDiatonic(int invertAxisdegrees, int keyRoot, Scale::Scales mode);

    static XformLambda makeQuantizePitchLambda(int keyRoot, Scale::Scales mode);

    int degreesInScale() const;

    /**
     * returns octave:degree from degree.
     */
    std::pair<int, int> normalizeDegree(int) const;

    /**
     * combine octave and degree into non-normalized degree
     */
    int octaveAndDegree(int octave, int degree);
    int octaveAndDegree(const ScaleRelativeNote&);
private:
    /**
     * To create a Scale, first you must new one,
     * then call init on it.
     * It's all a contructor / shared_from_this issue
     */
    Scale();
    void init(Scales scale, int keyRoot);
    
    /**
     * Handlers for the "in scale" operations for the case
     * when input is itself not in the scale
     */
    int transposeInScaleChromatic(int semitone, int scaleDegreesToTranspose) const;
    int invertInScaleChromatic(int semitone, int scaleDegreesToTranspose) const;

    /**
     *  make from semi-normalized semitones to srn
     * example: 0 is C. 11 is B. so B major would have 11, 13 ....
     */
    std::map<int, ScaleRelativeNotePtr> abs2srn;

    static std::vector<int> getBasePitches(Scales);
};