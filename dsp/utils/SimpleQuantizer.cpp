

#include "SimpleQuantizer.h"
#include "PitchUtils.h"


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
