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
    return ret;
}

void Triad::assertValid() const
{
    assertEQ(notes.size(), 3);
    for (auto note : notes) {
        assert(note != nullptr);
        assert(note->valid);
    }
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

TriadPtr Triad::make(ScalePtr scale, const ScaleRelativeNote& root, const Triad& previousTriad)
{
    TriadPtr best = make(scale, root, Inversion::Root);
    float bestPenalty = ratePair(scale, previousTriad, *best);
    printf("root penalty = %f\n", bestPenalty);

    TriadPtr candidate = make(scale, root, Inversion::First);
    float candidatePenalty = ratePair(scale, previousTriad, *candidate);

   printf("first penalty = %f\n", candidatePenalty);
    if (candidatePenalty < bestPenalty) {
        best = candidate;
        bestPenalty = candidatePenalty;
    }

    candidate = make(scale, root, Inversion::Second);
    candidatePenalty = ratePair(scale, previousTriad, *candidate);

   printf("seocnd penalty = %f\n", candidatePenalty);
    if (candidatePenalty < bestPenalty) {
        best = candidate;
        bestPenalty = candidatePenalty;
    }
    fflush(stdout);

    return best;
}

float Triad::ratePair(ScalePtr scale, const Triad& first, const Triad& second)
{
    float penalty = 0;

    const auto firstCvs = first.toCv(scale);
    const auto secondCvs = second.toCv(scale);
    if (isParallel(firstCvs, secondCvs)) {
        penalty += 10;
    }
    penalty += sumDistance(firstCvs, secondCvs);
    return penalty;
}

bool Triad::isParallel(const std::vector<float>& first, const std::vector<float>& second)
{
    assert(first.size() == 3 && second.size() == 3);
    const bool dir0 = first[0] > second[0];
    const bool dir1 = first[1] > second[1];
    const bool dir2 = first[2] > second[2];

    return dir0 == dir1 && dir1 == dir2;
}

float Triad::sumDistance(const std::vector<float>& first, const std::vector<float>& second)
{
    assert(first.size() == 3 && second.size() == 3);
    return std::abs(first[0] - second[0]) +
        std::abs(first[1] - second[1]) +
        std::abs(first[2] - second[2]);
}
