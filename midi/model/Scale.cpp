#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"

#include <assert.h>


Scale::Scale()
{
}

void Scale::init(Scales scale, int keyRoot)
{
    std::vector<int> notes = getBasePitches(scale);
    int degree = 0;
    for (auto it : notes) {

        int semi = keyRoot + it;
#if 0 // let's try not fully norm
        if (semi > 11) {
            semi -= 12;     // todo: use a util for this
        }
#endif
        // how do we come up with the 
        ScalePtr sc = getptr();
        ScaleRelativeNotePtr srn = std::make_shared<ScaleRelativeNote>(degree, 0, sc);
        abs2srn[semi] = srn;
        ++degree;
    }
}

ScalePtr Scale::getScale(Scale::Scales scale, int root)
{
  //  ScalePtr p = std::make_shared<Scale>();
    ScalePtr p(new Scale());

    p->init(scale, root);
    return ScalePtr(p);
}

#if 0   // this doesn't work with semi-normaled pitches
ScaleRelativeNotePtr Scale::getScaleRelativeNote(int semitone)
{
    assert(abs2srn.size());         // was this initialized?
    PitchUtils::NormP normP(semitone);

    auto it = abs2srn.find(normP.semi);
    if (it == abs2srn.end()) {
        // need to make an invalid one
        ScaleRelativeNotePtr ret(new ScaleRelativeNote());
        return ret;
    }
    return it->second;
}
#endif

ScaleRelativeNotePtr Scale::getScaleRelativeNote(int semitone)
{
    assert(abs2srn.size());         // was this initialized?
    PitchUtils::NormP normP(semitone);

    auto it = abs2srn.find(normP.semi);
    if (it != abs2srn.end()) {
        ScalePtr scale = shared_from_this();
        return ScaleRelativeNotePtr(new ScaleRelativeNote(it->second->degree, normP.oct, scale));
    }

    // since these are semi-normaled, lets try the next octave
    it = abs2srn.find(normP.semi + 12);
    if (it != abs2srn.end()) {
       
#if 0
        assert(normP.oct == 0);         // surely we need to adjust, also
                                       // in the bug case, oct -1 would be correct
       // I think we need the above correction, and -1 
       return it->second;
#else
        ScalePtr scale = shared_from_this();
        return ScaleRelativeNotePtr(new ScaleRelativeNote(it->second->degree, normP.oct - 1, scale));
#endif
    }

    // need to make an invalid one
    return ScaleRelativeNotePtr(new ScaleRelativeNote());
}

int Scale::getSemitone(const ScaleRelativeNote& note)
{
    // TODO: make smarter
    for (auto it : abs2srn) {
        int semi = it.first;
        ScaleRelativeNotePtr srn = it.second;
        if (srn->degree == note.degree) {
           // printf("found it!!\n");
           // assert(note.octave == 0);
            return semi + 12 * note.octave;
        }
    }
    //printf("didn't find it\n");
    return -1;
}


std::vector<int> Scale::getBasePitches(Scales scale)
{
    std::vector<int> ret;
    switch(scale) {
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
        default:
            assert(false);
     }
    return ret;

}