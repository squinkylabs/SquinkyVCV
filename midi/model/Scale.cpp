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
        if (semi > 11) {
            semi -= 12;     // todo: use a util for this
        }
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

int Scale::getSemitone(const ScaleRelativeNote& note)
{
    // TODO: make smarter
    for (auto it : abs2srn) {
        int semi = it.first;
        ScaleRelativeNotePtr srn = it.second;
        if (srn->isSameDegree(note)) {
            printf("found it!!\n");
            return semi;
        }
    }
    printf("didn't find it\n");
    return -1;
}


std::vector<int> Scale::getBasePitches(Scales scale)
{
    std::vector<int> ret;
    switch(scale) {
        case Scales::Major:
        ret = {0, 2, 4, 5, 7, 9, 11};
        break;
    default:
        assert(false);
    }
    return ret;

}