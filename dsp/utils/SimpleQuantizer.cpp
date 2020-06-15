

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

    switch (scale) {
    case Scales::_12Even:
        cur_set = pitches_12even;
        break;
    case Scales::_8Even:
        cur_set = pitches_8even;
        break;
    default:
        assert(false);
    }


}

#if 0
float SimpleQuantizer::quantize(float input)
{
    std::pair<int, int> x = PitchUtils::cvToPitch(input);
    int octave = x.first;
    int semi = x.second;
    
    //assert(semi == 0);

    // 0v is c4
    // this will only work for 12E
    return (float)octave - 4 + semi * PitchUtils::semitone;
}
#endif

float SimpleQuantizer::quantize(float _input)
{
    // This will quantize - dont' want that
   // std::pair<int, int> x = PitchUtils::cvToPitch(input);
  //  int octave = x.first;
  //  int semi = x.second;
    float octave = std::floor(_input);
    float fractionalInput = _input - octave;
    
    // let's find a table element less than or equal to us
    iterator itLow = cur_set.lower_bound(fractionalInput);
    iterator itHigh = itLow;
    if (*itLow > fractionalInput && itLow != cur_set.begin()) {
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

        const float deltaLo = fractionalInput - lo;
        const float deltaHi = hi - fractionalInput;
        assert(deltaLo >= 0);
        assert(deltaHi >= 0);
        
        fraction = (deltaHi < deltaLo) ? hi : lo;
    }

    return (float)octave + fraction;



    
}