

#include "SimpleQuantizer.h"
#include "PitchUtils.h"
#include <vector>



SimpleQuantizer::SimpleQuantizer(std::vector<SimpleQuantizer::Scales>& scales, SimpleQuantizer::Scales scale)
{
    const float  s = PitchUtils::semitone;
    // fill 12 even
    for (int i = 0; i <= 12; ++i) {
        pitches_12even.insert(i * s);
    }

    // fill 8 even
    pitches_8even.insert(0 * s);        // C
    pitches_8even.insert(2 * s);        // D
    pitches_8even.insert(4 * s);        // E
    pitches_8even.insert(5 * s);        // F
    pitches_8even.insert(7 * s);        // G
    pitches_8even.insert(9 * s);        // A
    pitches_8even.insert(11 * s);        // B
    pitches_8even.insert(12 * s);        // C2

    //
    pitches_8just.insert(1.f/1 - 1);
    pitches_8just.insert(9.f/8 - 1);
    pitches_8just.insert(5.f /4 - 1);
    pitches_8just.insert(4.f /3 - 1);
    pitches_8just.insert(3.f /2 - 1);
    pitches_8just.insert(5.f /3 - 1);
    pitches_8just.insert(15.f /8 - 1);
    pitches_8just.insert(2.f/1 - 1);

    pitches_12just.insert(1.f/1 - 1);
    pitches_12just.insert(16.f / 15 - 1);
    pitches_12just.insert(9.f / 8 - 1);
    pitches_12just.insert(6.f / 5 - 1);
    pitches_12just.insert(5.f / 4 - 1);
    pitches_12just.insert(4.f / 3 - 1);
    pitches_12just.insert(45.f / 32 - 1);
    pitches_12just.insert(3.f / 2 - 1);
    pitches_12just.insert(8.f / 5 - 1);
    pitches_12just.insert(5.f / 3 - 1);
    pitches_12just.insert(9.f / 5 - 1);
    pitches_12just.insert(15.f / 18 - 1);
    pitches_12just.insert(2.f / 1 - 1);

    setScale(scale);

    assert(pitches_12even.size() == 12+1);
    assert(pitches_12just.size() == 12+1);
    assert(pitches_8even.size() == 8);
    assert(pitches_8just.size() == 8);

}

float SimpleQuantizer::quantize(float _input)
{
    if (!cur_set) {
        return _input;
    }
    float octave = std::floor(_input);
    float fractionalInput = _input - octave;
    
    // let's find a table element less than or equal to us
    iterator itLow = cur_set->lower_bound(fractionalInput);
    iterator itHigh = itLow;
    if (*itLow > fractionalInput && itLow != cur_set->begin()) {
        --itLow;
    }
    assert(*itLow <= fractionalInput);
    assert(*itHigh >= fractionalInput);

    float fraction = 0;
    if (itLow == itHigh) {
        fraction = *itLow;
    } else {
        const float lo = *itLow;
        const float hi = *itHigh;

        const float slopAmount = PitchUtils::semitone * .1f;     // if we are right in between, favor rounding down
        const float deltaLo = fractionalInput - lo;
        const float deltaHi = hi - fractionalInput;
        assert(deltaLo >= 0);
        assert(deltaHi >= 0);

        // if we are super close to the middle, round down.
        if (std::abs(deltaHi - deltaLo) < slopAmount) {
            fraction = lo;
        }
        else {
            fraction = (deltaHi < deltaLo) ? hi : lo;
        }
    }

    return (float)octave + fraction;
}


void SimpleQuantizer::setScale(SimpleQuantizer::Scales scale)
{
    switch(scale) {
        case  Scales::_12Even:
            cur_set = &pitches_12even;
            break;
        case  Scales::_8Even:
            cur_set = &pitches_8even;
            break;
        case Scales::_12Just:
            cur_set = &pitches_12just;
            break;
        case  Scales::_8Just:
            cur_set = &pitches_8just;
            break;
        case Scales::_off:
            cur_set = nullptr;
            break;
        default:
            assert(false);
    }
 
    assert(!cur_set || !cur_set->empty());
}