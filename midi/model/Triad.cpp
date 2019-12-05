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
