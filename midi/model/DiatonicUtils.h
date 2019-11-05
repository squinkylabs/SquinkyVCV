#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>

class MidiEvent;
using MidiEventPtr = std::shared_ptr<MidiEvent>;

/**
 * Pitch representation: c is zero.
 * Many functions have two forms:
 *  xxxC() which assumes C Major
 *  xxx(root, mode) which does not.
 */
class DiatonicUtils
{
public:

    /*****************************************************************
     * Constants for the 12 pitches in a chromatic scale
     */
    static const int c = {0};
    static const int c_ = {1};
    static const int d = {2};
    static const int d_ = {3};
    static const int e = {4};
    static const int f = {5};
    static const int f_ = {6};
    static const int g = {7};
    static const int g_ = {8};
    static const int a = {9};
    static const int a_ = {10};
    static const int b = {11};

    /**
     * can handle pitches 0..23
     */
    static std::string getPitchString(int pitch);

    enum class Modes
    {
        Major,
        Dorian,
        Phrygian,
        Lydian,
        Mixolydian,
        Minor,
        Locrian
    };


    /*****************************************************************
     * Utilities for C Major
     */
    static bool isNoteInC(int pitch);

    /**
     * Note must be scale degree.
     * return is 0 for root, 1 for second...
     */
    static int getScaleDegreeInC(int pitch);

    static int getScaleDegree(int pitch, int key, Modes mode);

    /**
     * Gets a vector of pitch offsets for transpose.
     * @param transposeAmount is in semitones. must be >= 0 and <= 11.
     * @param quantize forces all notes to be in scale. when it's false scale notes stay, but non-scale notes
     *      can go to non-scale pitches
     * @returns transpose semitones
     */
    static std::vector<int> getTransposeInC(int transposeAmount, bool quantize);
    static std::vector<int> getTransposeInCQuantized(int _transposeAmount);
    static std::vector<int> getTransposeInCInformed(int _transposeAmount);

    //static std::vector<int> getInvertInCInformed(int invertAxis);


    /**
     * Applies transposes to the 12 semitones of chromatic C scale.
     * Forces output to be in C major
     */
    static std::vector<int> getTransposedPitchesInC(const std::vector<int>& tranposes);

    /**
     * converts a chromatic transpose to the "nearest" scale degree in C
     */
    static int quantizeXposeToScaleDegreeInC(int xpose);

    /** 
     * accepts degree from 0..12 (two octaves
     */
    static int getPitchFromScaleDegreeInC(int degree);
    static int getPitchFromScaleDegreeInCMinor(int degree);
    static int getPitchFromScaleDegreeInCDorian(int degree);
    static int getPitchFromScaleDegreeInCPhrygian(int degree);
    static int getPitchFromScaleDegreeInCLydian(int degree);
    static int getPitchFromScaleDegreeInCMixolydian(int degree);
    static int getPitchFromScaleDegreeInCLocrian(int degree);



    static int getPitchFromScaleDegree(int degree, int key, Modes mode);
   
    

    /*****************************************************************
     * Misc util
     */

    static void _dumpTransposes(const char*, const std::vector<int>&);

    /**
     * returns octave:semi from semi, where semi 0 == C
     */
    static std::pair<int, int> normalizePitch(int);


    /********************** not constrained to C ***************************/

    /**
     * Applies transposes to the 12 semitones of chromatic C scale.
     * Forces output to be in mode.
     */
    static std::vector<int> getTranspose(int transposeAmount, int keyRoot, Modes mode, bool quantize);
  //  static std::vector<int> getInvert(int invertAxis, int keyRoot, Modes mode);

    /**
     * Computes relative pitch offset of a mode relative to CMajor.
     * Example: A Minor.
     * All the notes in A minor are already in C Major, 
     * since A is the relative minor of C. So the offset is zero.
     *
     * Example: C Minor.
     * If we transpose +9, we get to A minor, which is CMajor.
     *
     */
    static int getPitchOffsetRelativeToCMaj(int keyRoot, Modes mode);

    /**
     * returns the number of semitones you must transpose up to get to relative Major
     */
    static int getOffsetToRelativeMaj(Modes mode);

    /********************* xform lambdas ******************************/

    static std::function<void(MidiEventPtr)> makeTransposeLambda(int transposeSemitones, bool constrainToKeysig, int keyRoot, Modes mode);
    static std::function<void(MidiEventPtr)> makeInvertLambda(int invertAxisSemitones, bool constrainToKeysig, int keyRoot, Modes mode);
    static std::function<void(MidiEventPtr)> makeInvertLambdaChromatic(int invertAxisSemitones);
    static std::function<void(MidiEventPtr)> makeInvertLambdaDiatonic(int invertAxisSemitones, int keyRoot, DiatonicUtils::Modes mode);

    /**
     * puts input in the range 0..end-1 by 
     * repeatedly adding or subtracing end
     */
    static int normalize(int input, int end);

    static int normalizeDegree(int degree);
};