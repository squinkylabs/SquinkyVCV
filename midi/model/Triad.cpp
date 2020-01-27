#include "PitchUtils.h"
#include "Scale.h"
#include "ScaleRelativeNote.h"
#include "Triad.h"

#include "asserts.h"

Triad::Triad() : notes(3)
{

}

TriadPtr Triad::make(ScalePtr scale, const ScaleRelativeNote& root, Triad::Inversion inversion)
{
    TriadPtr ret =  TriadPtr(new Triad());
    auto rootClone = Scale::clone(root);
    ret->notes[0] = rootClone;
    ret->notes[1] = scale->transposeDegrees(root, 2);
    ret->notes[2] = scale->transposeDegrees(root, 4);
    switch (inversion) {
        case Inversion::Root:
            break;
        case Inversion::First:
            ret->notes[1] = scale->transposeOctaves(*ret->notes[1], -1);
            break;
        case Inversion::Second:
            ret->notes[2] = scale->transposeOctaves(*ret->notes[2], -1);
            break;
        default:
            assert(false);

    }
    ret->sort(scale);
    return ret;
}

void Triad::sort(ScalePtr scale)
{
    auto cv = this->toCv(scale);
    if (cv[0] > cv[1]) {
        std::swap(notes[0], notes[1]);
    }

    if (cv[1] > cv[2]) {
        std::swap(notes[1], notes[2]);
    }

    cv = this->toCv(scale);
    if (cv[0] > cv[1]) {
        std::swap(notes[0], notes[1]);
    }

    if (cv[1] > cv[2]) {
        std::swap(notes[1], notes[2]);
    }
}

void Triad::assertValid(ScalePtr scale) const
{
    assertEQ(notes.size(), 3);
    for (auto note : notes) {
        assert(note != nullptr);
        assert(note->valid);
    }
    
    auto cv = this->toCv(scale);
    assert(cv[0] < cv[1]);
    assert(cv[1] < cv[2]);
}

void Triad::transposeOctave(ScalePtr scale, int index, int octave)
{
    auto srn = notes[index];
    notes[index] = scale->transposeOctaves(*srn, octave);
}

std::vector<float> Triad::toCv(ScalePtr scale) const
{
    std::vector<float> ret;
    int index = 0;
    for (auto srn : notes) {
        float pitchCV = scale->getPitchCV(*this->get(index));
        ret.push_back(pitchCV);
        ++index;
    }
    return ret;
}

TriadPtr Triad::make(ScalePtr scale, const ScaleRelativeNote& root, const Triad& previousTriad, bool searchOctaves)
{
    return searchOctaves ?
        makeOctaves(scale, root, previousTriad) :
        makeNorm(scale, root, previousTriad);
}

TriadPtr Triad::makeOctaves(ScalePtr scale, const ScaleRelativeNote& root, const Triad& previousTriad)
{
    TriadPtr best = make(scale, root, Inversion::Root);
    float bestPenalty = ratePair(scale, previousTriad, *best);

    const int beginOctave = -1;
    const int endOctave = 1;
    for (int rootOctave = beginOctave; rootOctave <= endOctave; ++rootOctave) {
        for (int thirdOctave = beginOctave; thirdOctave <= endOctave; ++thirdOctave) {
            for (int fifthOctave = beginOctave; fifthOctave <= endOctave; ++fifthOctave)
            {
               // for (int iinv = int(Inversion::Root); iinv <= int(Inversion::Second); ++iinv) {
                 //   Inversion inv = Inversion(iinv);
                    TriadPtr candidate = make(scale, root, Inversion::Root);
                    candidate->transposeOctave(scale, 0, rootOctave);
                    candidate->transposeOctave(scale, 1, thirdOctave);
                    candidate->transposeOctave(scale, 2, fifthOctave);
                    float candidatePenalty = ratePair(scale, previousTriad, *candidate);
                  //  printf("root penalty oct %d = %.2f\n", octave, candidatePenalty);
                    if (candidatePenalty < bestPenalty) {
                        best = candidate;
                        bestPenalty = candidatePenalty;
                    }
                //}
            }
        }
    }
    

    return best;
}



TriadPtr Triad::makeNorm(ScalePtr scale, const ScaleRelativeNote& root, const Triad& previousTriad)
{
    TriadPtr best = make(scale, root, Inversion::Root);
    float bestPenalty = ratePair(scale, previousTriad, *best);


    //ScaleRelativeNotePtr root = scale->transposeOctaves(_root, octave);

    TriadPtr candidate = make(scale, root, Inversion::Root);
    float candidatePenalty = ratePair(scale, previousTriad, *candidate);
  
    if (candidatePenalty < bestPenalty) {
        best = candidate;
        bestPenalty = candidatePenalty;
    }


    candidate = make(scale, root, Inversion::First);
    candidatePenalty = ratePair(scale, previousTriad, *candidate);

    if (candidatePenalty < bestPenalty) {
        best = candidate;
        bestPenalty = candidatePenalty;
    }

    candidate = make(scale, root, Inversion::Second);
    candidatePenalty = ratePair(scale, previousTriad, *candidate);

    if (candidatePenalty < bestPenalty) {
        best = candidate;
        bestPenalty = candidatePenalty;
    }

    printf("\nleaving make norm. final rating prev.next\n");
    previousTriad._dump("prev", scale);
    best->_dump("best", scale);
    ratePair(scale, previousTriad, *best);
    printf("\n");
    fflush(stdout);


    return best;
}


float Triad::ratePair(ScalePtr scale, const Triad& first, const Triad& second)
{
    float penalty = 0;

    const auto firstCvs = first.toCv(scale);
    const auto secondCvs = second.toCv(scale);
    if (isParallel(firstCvs, secondCvs)) {
        penalty += 5;           // 10 seemed too high
        printf("rate pair penalty for parallel 5\n");
    }
    penalty += sumDistance(firstCvs, secondCvs);
    printf("penalty for distance = %f span = %f\n", sumDistance(firstCvs, secondCvs), secondCvs[2] - secondCvs[0]);
    return penalty;
}

bool Triad::isParallel(const std::vector<float>& first, const std::vector<float>& second)
{
    assert(first.size() == 3 && second.size() == 3);
    const bool up0 = first[0] < second[0];
    const bool up1 = first[1] < second[1];
    const bool up2 = first[2] < second[2];

    const bool dn0 = first[0] > second[0];
    const bool dn1 = first[1] > second[1];
    const bool dn2 = first[2] > second[2];

    bool ret = (up0 && up1 && up2) || (dn0 && dn1 && dn2);
#if 0
    printf("isPar (%.2f, %.2f) (%.2f, %.2f) (%.2f, %.2f) ret = %d\n",
        first[0], second[0],
        first[1], second[1],
        first[2], second[2],
        ret);
#endif

    return ret;
}

float Triad::sumDistance(const std::vector<float>& first, const std::vector<float>& second)
{
    assert(first.size() == 3 && second.size() == 3);
    return std::abs(first[0] - second[0]) +
        std::abs(first[1] - second[1]) +
        std::abs(first[2] - second[2]);
}

 void Triad::_dump(const char* title, ScalePtr scale) const
 {
     auto pitches = this->toCv(scale);
     printf("triad %s = %.2f, %.2f, %.2f\n", title, pitches[0], pitches[1], pitches[2]);
 }
