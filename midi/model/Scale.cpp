#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"
#include "SqMidiEvent.h"

#include <assert.h>


Scale::Scale()
{
}

void Scale::init(Scales scale, int keyRoot)
{
  
    std::vector<int> notes = getBasePitches(scale);
    int degree = 0;
    for (auto it : notes) {
        const int semi = keyRoot + it;
     //  ScaleRelativeNotePtr srn = std::make_shared<ScaleRelativeNote>(degree, 0);
        ScaleRelativeNotePtr srn(new ScaleRelativeNote(degree, 0));
        abs2srn[semi] = srn;
        ++degree;
    }
}

ScalePtr Scale::getScale(Scale::Scales scale, int root)
{
    ScalePtr p(new Scale());

    p->init(scale, root);
    return ScalePtr(p);
}

ScaleRelativeNotePtr Scale::getScaleRelativeNotePtr(int semitone) const
{
    auto srn = getScaleRelativeNote(semitone);
    ScaleRelativeNote* rawNote = nullptr;


    rawNote = (srn.valid) ? new ScaleRelativeNote(srn.degree, srn.octave) : new ScaleRelativeNote();

    return ScaleRelativeNotePtr(rawNote);
  //  ScaleRelativeNotePtr ret(rawNote);
  //  return ret;
}

ScaleRelativeNotePtr Scale::clone(const ScaleRelativeNote& note)
{
    assert(note.valid);
    return ScaleRelativeNotePtr(new ScaleRelativeNote(note.degree, note.octave));
}

ScaleRelativeNotePtr Scale::transposeDegrees(const ScaleRelativeNote& note, int degrees)
{
    assert(note.valid);
    int octaveAndDegree = this->octaveAndDegree(note);
    octaveAndDegree += degrees;
    std::pair<int, int> normResult = normalizeDegree(octaveAndDegree);
    return ScaleRelativeNotePtr(new ScaleRelativeNote(normResult.second, normResult.first));
}

ScaleRelativeNotePtr Scale::transposeOctaves(const ScaleRelativeNote& note, int octaves)
{
    assert(note.valid);
    int octaveAndDegree = this->octaveAndDegree(note);
    octaveAndDegree += this->degreesInScale() * octaves;
    std::pair<int, int> normResult = normalizeDegree(octaveAndDegree);
    return ScaleRelativeNotePtr(new ScaleRelativeNote(normResult.second, normResult.first));
}


ScaleRelativeNote Scale::getScaleRelativeNote(int semitone) const
{
    assert(abs2srn.size());         // was this initialized?
    PitchUtils::NormP normP(semitone);

    auto it = abs2srn.find(normP.semi);
    if (it != abs2srn.end()) {
        return ScaleRelativeNote(it->second->degree, normP.oct);
    }

    // since these are semi-normaled, lets try the next octave
    it = abs2srn.find(normP.semi + 12);
    if (it != abs2srn.end()) {
        return ScaleRelativeNote(it->second->degree, normP.oct - 1);
    }

    // return an invalid one
    return ScaleRelativeNote();
}

float Scale::getPitchCV(const ScaleRelativeNote& srn) const
{
    int semi = this->getSemitone(srn);
    float pitchCV = PitchUtils::semitoneToCV(semi);
    return pitchCV;
}
    

int Scale::getSemitone(const ScaleRelativeNote& note) const
{
    // TODO: make smarter
    for (auto it : abs2srn) {
        int semi = it.first;
        ScaleRelativeNotePtr srn = it.second;
        if (srn->degree == note.degree) {
            return semi + 12 * note.octave;
        }
    }
    return -1;
}


std::vector<int> Scale::getBasePitches(Scales scale)
{
    std::vector<int> ret;
    switch (scale) {
        case Scales::Major:
            ret = {0, 2, 4, 5, 7, 9, 11};
            break;
        case Scales::Minor:
            ret = {0, 2, 3, 5, 7, 8, 10};
            break;
        case Scales::Phrygian:
            ret = {0, 1, 3, 5, 7, 8, 10};
            break;
        case Scales::Mixolydian:
            ret = {0, 2, 4, 5, 7, 9, 10};
            break;
        case Scales::Locrian:
            ret = {0, 1, 3, 5, 6, 8, 10};
            break;
        case Scales::Lydian:
            ret = {0, 2, 4, 6, 7, 9, 11};
            break;
        case Scales::Dorian:
            ret = {0, 2, 3, 5, 7, 9, 10};
            break;
        case Scales::MinorPentatonic:
            ret = {0, 3, 5, 7, 10};
            break;
        case Scales::HarmonicMinor:
            ret = {0, 2, 3, 5, 7, 8, 11};
            break;
        case Scales::Diminished:
            ret = {0, 2, 3, 5, 6, 8, 9, 11};
            break;
        case Scales::DominantDiminished:
            ret = {0, 1, 3, 4, 6, 7, 9, 10};
            break;
        case Scales::WholeStep:
            ret = {0, 2, 4, 6, 8, 10};
            break;
        default:
            assert(false);
    }
    return ret;
}

int Scale::degreesInScale() const
{
    return int(abs2srn.size());
}

std::pair<int, int> Scale::normalizeDegree(int degree) const
{
    int octave = 0;
    while (degree >= degreesInScale()) {
        degree -= degreesInScale();
        octave++;
    }

    while (degree < 0) {
        degree += degreesInScale();
        octave--;
    }
    return std::make_pair(octave, degree);
}

 int Scale::octaveAndDegree(int octave, int degree)
 {
     return degree + octave * degreesInScale();
 }

  int Scale::octaveAndDegree(const ScaleRelativeNote& srn)
 {
     return octaveAndDegree(srn.octave, srn.degree);
 }

int Scale::invertInScale(int semitone, int inversionAxisDegree) const
{
    auto srn = this->getScaleRelativeNote(semitone);
    if (!srn.valid) {
        return invertInScaleChromatic(semitone, inversionAxisDegree);
    }

    int inputDegreeAbs = srn.degree + srn.octave * this->degreesInScale();

    int invertedDegreesAbs = 2 * inversionAxisDegree - inputDegreeAbs;

    auto normalizedInvertedDegreesAbs = normalizeDegree(invertedDegreesAbs);

    ScaleRelativeNote srnInverted(normalizedInvertedDegreesAbs.second, normalizedInvertedDegreesAbs.first);
    const int invertedSemitones = this->getSemitone(srnInverted);

    return invertedSemitones;
}

int Scale::transposeInScale(int semitone, int scaleDegreesToTranspose) const
{
    auto srn = this->getScaleRelativeNote(semitone);
    if (!srn.valid) {
        return transposeInScaleChromatic(semitone, scaleDegreesToTranspose);
    }

    int transposedOctave = srn.octave;
    int transposedDegree = srn.degree;

    transposedDegree += scaleDegreesToTranspose;
    auto normalizedDegree = normalizeDegree(transposedDegree);

    transposedOctave += normalizedDegree.first;
    transposedDegree = normalizedDegree.second;


   // auto srn2 = std::make_shared<ScaleRelativeNote>(transposedDegree, transposedOctave);
    ScaleRelativeNote srn2(transposedDegree, transposedOctave);
    return this->getSemitone(srn2);
}

int Scale::quantizeToScale(int semitone) const 
{
    auto srn1 = this->getScaleRelativeNote(semitone);
    if (srn1.valid) {
        return semitone;
    }

    auto srn2 = this->getScaleRelativeNote(semitone - 1);
    if (srn2.valid) {
        return semitone - 1;
    }

    auto srn3 = this->getScaleRelativeNote(semitone + 1);
    if (srn3.valid) {
        return semitone + 1;
    }

    assert(false);
    return 0; 
}

int Scale::transposeInScaleChromatic(int _semitone, int scaleDegreesToTranspose) const
{
    assert(!getScaleRelativeNote(_semitone).valid);

    int lowerSemitone = _semitone-1;
    int higherSemitone = _semitone+1;

    // search for scale relative that bracket us.
    // Note that in some scales these can be a whole step away
    auto srnPrev = getScaleRelativeNotePtr(lowerSemitone);
    if (!srnPrev->valid) {
        lowerSemitone--;
        srnPrev = getScaleRelativeNotePtr(lowerSemitone);
    }

    auto srnNext = getScaleRelativeNotePtr(higherSemitone);
    if (!srnNext->valid) {
        higherSemitone++;
        srnNext = getScaleRelativeNotePtr(higherSemitone);
    }

    assert(srnPrev->valid && srnNext->valid);


    // If we can fit between these, we will.
    // If not, we will always round down.
    const int transposePrev = transposeInScale(lowerSemitone, scaleDegreesToTranspose);
    const int transposeNext = transposeInScale(higherSemitone, scaleDegreesToTranspose);
    return (transposePrev + transposeNext) / 2;
}

int Scale::invertInScaleChromatic(int _semitone, int inversionDegree) const
{
    assert(!getScaleRelativeNote(_semitone).valid);

    int lowerSemitone = _semitone - 1;
    int higherSemitone = _semitone + 1;
 
    // search for scale relative that bracket us.
    // Note that in some scales these can be a whole step away
    auto srnPrev = getScaleRelativeNotePtr(lowerSemitone);
    if (!srnPrev->valid) {
        lowerSemitone--;
        srnPrev = getScaleRelativeNotePtr(lowerSemitone);
    }

    auto srnNext = getScaleRelativeNotePtr(higherSemitone);
    if (!srnNext->valid) {
        higherSemitone++;
        srnNext = getScaleRelativeNotePtr(higherSemitone);
    }

    // For all the scales we have so far, notes out of scale are
    // always surrounded by notes in scale. Not true for all, however.
    assert(srnPrev->valid && srnNext->valid);


    // If we can fit between these, we will.
    // If now, we will always round down.
    const int invertPrev = invertInScale(lowerSemitone, inversionDegree);
    const int invertNext = invertInScale(higherSemitone, inversionDegree);
    return (invertPrev + invertNext) / 2;
}


XformLambda Scale::makeTransposeLambdaChromatic(int transposeSemitones)
{
    const float delta = transposeSemitones * PitchUtils::semitone;
    return [delta](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->pitchCV += delta;
        }
    };
}

XformLambda Scale::makeInvertLambdaChromatic(int invertAxisSemitones)
{
    const float axis = PitchUtils::semitoneToCV(invertAxisSemitones);
    return [axis](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->pitchCV = 2 * axis - note->pitchCV;
        }
    };
}

XformLambda Scale::makeQuantizePitchLambda(int keyRoot, Scale::Scales mode)
{
    ScalePtr scale = Scale::getScale(mode, keyRoot);
    return [scale](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            const int semitone = PitchUtils::cvToSemitone(note->pitchCV);
            const int quantizedSemi = scale->quantizeToScale(semitone);
            note->pitchCV = PitchUtils::semitoneToCV(quantizedSemi);
        }      
    };
}

XformLambda Scale::makeTransposeLambdaScale(int scaleDegrees, int keyRoot, Scales mode)
{
    ScalePtr scale = Scale::getScale(mode, keyRoot);
    return[scale, scaleDegrees](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            const int semitone = PitchUtils::cvToSemitone(note->pitchCV);
            const int xposedSemi = scale->transposeInScale(semitone, scaleDegrees);
            note->pitchCV = PitchUtils::semitoneToCV(xposedSemi);
        }
    };
}

XformLambda Scale::makeInvertLambdaDiatonic(int invertAxisdegrees, int keyRoot, Scale::Scales mode)
{
    ScalePtr scale = Scale::getScale(mode, keyRoot);
    return [scale, invertAxisdegrees](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            const int semitone = PitchUtils::cvToSemitone(note->pitchCV);
            const int invertedSemi = scale->invertInScale(semitone, invertAxisdegrees);
            note->pitchCV = PitchUtils::semitoneToCV(invertedSemi);
        }
    };
}